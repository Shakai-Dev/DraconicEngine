module;

#include <print>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <functional>
#include <atomic>
#include <thread>
#include <vector>
#include <cstring>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <filesystem>
#include <memory>
#include <source_location>
#include <system_error>

module core.logging;

import core.stdtypes;

namespace draco::core::logging
{
    static constexpr usize QUEUE_CAPACITY = 8192;
    static constexpr usize QUEUE_MASK = QUEUE_CAPACITY - 1;

    struct Node
    {
        std::atomic<usize> sequence;
        LogMessage msg;
    };

    alignas(64) static Node g_queue[QUEUE_CAPACITY];
    alignas(64) static std::atomic<usize> g_enqueue_pos{0};
    alignas(64) static std::atomic<usize> g_dequeue_pos{0};
    alignas(64) static std::atomic<usize> g_dispatched_pos{0};

    static std::atomic<u64> g_dropped_count{0};
    static std::atomic<u64> g_written_count{0};

    static std::atomic<bool> g_running{false};
    static std::atomic<bool> g_accepting_logs{false};
    static std::atomic<u64> g_active_producers{0};

    static std::thread g_worker_thread;
    static std::mutex g_worker_mutex;
    static std::condition_variable g_worker_cv;

    static std::atomic<LogVerbosity> g_global_verbosity{LogVerbosity::Info};

    struct SinkList
    {
        std::vector<std::shared_ptr<ILogSink>> sinks;
    };

    static std::atomic<std::shared_ptr<SinkList>> g_sink_list{ nullptr };
    static std::mutex g_sink_update_mutex;
    static std::mutex g_sink_io_mutex;
    static std::atomic<uint64_t> g_roll_seq{0};

    static std::chrono::steady_clock::time_point g_start_time;

    namespace internal
    {
        void (*fatal_hook)() = []() { std::abort(); };
    }

    static u32 get_thread_id()
    {
        thread_local u32 cached_id = static_cast<u32>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
        return cached_id;
    }

    static u64 uptime_ns()
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - g_start_time).count();
    }

    static const char* verbosity_to_string(LogVerbosity v)
    {
        switch (v)
        {
            case LogVerbosity::Fatal:       return "FATAL";
            case LogVerbosity::Error:       return "ERROR";
            case LogVerbosity::Warning:     return "WARNING";
            case LogVerbosity::Display:     return "DISPLAY";
            case LogVerbosity::Info:        return "INFO";
            case LogVerbosity::Verbose:     return "VERBOSE";
            case LogVerbosity::VeryVerbose: return "VERYVERBOSE";
        }
        return "UNKNOWN";
    }

    static void enqueue(const LogMessage& msg)
    {
        usize pos = g_enqueue_pos.load(std::memory_order_relaxed);
        while (true)
        {
            Node* node = &g_queue[pos & QUEUE_MASK];
            usize seq = node->sequence.load(std::memory_order_acquire);
            intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos);

            if (diff == 0)
            {
                if (g_enqueue_pos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
                {
                    node->msg = msg;
                    node->msg.sequence_id = pos; 
                    
                    bool was_empty = (pos == g_dequeue_pos.load(std::memory_order_relaxed));
                    
                    node->sequence.store(pos + 1, std::memory_order_release);
                    
                    if (was_empty)
                    {
                        std::lock_guard lock(g_worker_mutex);
                        g_worker_cv.notify_one();
                    }
                    return;
                }
            }
            else if (diff < 0)
            {
                g_dropped_count.fetch_add(1, std::memory_order_relaxed);
                return;
            }
            else
            {
                pos = g_enqueue_pos.load(std::memory_order_relaxed);
            }
        }
    }

    static bool dequeue(LogMessage& msg)
    {
        usize pos = g_dequeue_pos.load(std::memory_order_relaxed);
        Node* node = &g_queue[pos & QUEUE_MASK];
        usize seq = node->sequence.load(std::memory_order_acquire);
        intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1);

        if (diff == 0)
        {
            msg = node->msg;
            g_dequeue_pos.store(pos + 1, std::memory_order_relaxed);
            node->sequence.store(pos + QUEUE_CAPACITY, std::memory_order_release);
            return true;
        }
        return false;
    }

    static void worker_loop()
    {
        while (true)
        {
            bool running = g_running.load(std::memory_order_acquire);
            std::shared_ptr<SinkList> current_sinks = g_sink_list.load(std::memory_order_acquire);

            LogMessage msg;
            bool popped = false;
            
            while (dequeue(msg))
            {
                if (current_sinks)
                {
                    for (auto& sink : current_sinks->sinks)
                    {
                        if (sink) { 
                            std::lock_guard<std::mutex> io_lock(g_sink_io_mutex); 
                            sink->write(msg); 
                        }
                    }
                }
                g_written_count.fetch_add(1, std::memory_order_relaxed);
                g_dispatched_pos.store(g_dequeue_pos.load(std::memory_order_relaxed), std::memory_order_release);
                popped = true;
            }

            if (!running && !popped)
            {
                if (g_dequeue_pos.load(std::memory_order_relaxed) == g_enqueue_pos.load(std::memory_order_acquire))
                {
                    break;
                }
            }

            if (!popped)
            {
                std::unique_lock<std::mutex> lock(g_worker_mutex);
                if (!g_running.load(std::memory_order_acquire)) 
                    continue;
                if (g_dequeue_pos.load(std::memory_order_relaxed) != g_enqueue_pos.load(std::memory_order_acquire)) 
                    continue;

                g_worker_cv.wait_for(lock, std::chrono::milliseconds(2));
            }
        }
    }

    void init()
    {
        if (g_worker_thread.joinable()) { 
            return; 
        }

        g_start_time = std::chrono::steady_clock::now();
        g_global_verbosity.store(LogVerbosity::Info);
        g_dropped_count.store(0);
        g_written_count.store(0);
        g_active_producers.store(0);

        for (usize i = 0; i < QUEUE_CAPACITY; i++)
        {
            g_queue[i].sequence.store(i, std::memory_order_relaxed);
        }
        g_enqueue_pos.store(0);
        g_dequeue_pos.store(0);
        g_dispatched_pos.store(0);

        {
            std::lock_guard lock(g_sink_update_mutex);
            g_sink_list.store(std::make_shared<SinkList>(), std::memory_order_release);
        }

        g_accepting_logs.store(true, std::memory_order_release);
        g_running.store(true, std::memory_order_release);
        g_worker_thread = std::thread(worker_loop);
    }

    void shutdown()
    {
        if (g_accepting_logs.exchange(false, std::memory_order_release))
        {
            while (g_active_producers.load(std::memory_order_acquire) > 0)
            {
                std::this_thread::yield();
            }

            flush();

            g_running.store(false, std::memory_order_release);
            {
                std::lock_guard lock(g_worker_mutex);
                g_worker_cv.notify_all();
            }
            if (g_worker_thread.joinable())
            {
                g_worker_thread.join();
            }

            std::lock_guard lock(g_sink_update_mutex);
            g_sink_list.store(nullptr, std::memory_order_release);
        }
    }

    void flush(std::chrono::milliseconds timeout)
    {
        auto start = std::chrono::steady_clock::now();
        usize target = g_enqueue_pos.load(std::memory_order_acquire);
        while (g_dispatched_pos.load(std::memory_order_acquire) < target)
        {
            if (timeout != std::chrono::milliseconds(0) && (std::chrono::steady_clock::now() - start) >= timeout)
            {
                break;
            }
            std::this_thread::yield();
        }

        std::shared_ptr<SinkList> current_sinks = g_sink_list.load(std::memory_order_acquire);
        if (current_sinks)
        {
            for (auto& sink : current_sinks->sinks)
            {
                if (sink) { 
                    if (timeout != std::chrono::milliseconds(0))
                    {
                        std::unique_lock<std::mutex> io_lock(g_sink_io_mutex, std::defer_lock);
                        while (!io_lock.try_lock())
                        {
                            if ((std::chrono::steady_clock::now() - start) >= timeout)
                                break;
                            std::this_thread::yield();
                        }
                        if (io_lock.owns_lock())
                        {
                            sink->flush(); 
                        }
                    }
                    else
                    {
                        std::lock_guard<std::mutex> io_lock(g_sink_io_mutex); 
                        sink->flush(); 
                    }
                }
            }
        }
    }

    void set_global_verbosity(LogVerbosity v)
    {
        g_global_verbosity.store(v);
    }

    void add_sink(std::shared_ptr<ILogSink> sink)
    {
        if (!sink) return;
        std::lock_guard lock(g_sink_update_mutex);
        auto current = g_sink_list.load(std::memory_order_acquire);
        auto next = std::make_shared<SinkList>();
        if (current)
        {
            next->sinks = current->sinks;
        }
        if (std::find(next->sinks.begin(), next->sinks.end(), sink) == next->sinks.end())
        {
            next->sinks.push_back(sink);
        }
        g_sink_list.store(next, std::memory_order_release);
    }

    void remove_sink(std::shared_ptr<ILogSink> sink)
    {
        if (!sink) return;
        std::lock_guard lock(g_sink_update_mutex);
        auto current = g_sink_list.load(std::memory_order_acquire);
        auto next = std::make_shared<SinkList>();
        if (current)
        {
            next->sinks = current->sinks;
        }
        auto it = std::find(next->sinks.begin(), next->sinks.end(), sink);
        if (it != next->sinks.end())
        {
            next->sinks.erase(it);
        }
        g_sink_list.store(next, std::memory_order_release);
    }

    namespace internal 
    {
        bool is_verbosity_enabled(LogVerbosity verbosity, const LogCategory& category) noexcept
        {
            LogVerbosity target_verbosity = g_global_verbosity.load(std::memory_order_relaxed);
            i32 category_override = category.override_verbosity.load(std::memory_order_relaxed);
            if (category_override >= 0)
            {
                target_verbosity = static_cast<LogVerbosity>(category_override);
            }
            return verbosity <= target_verbosity;
        }

        void enqueue_log(LogVerbosity verbosity, const LogCategory& category, const std::source_location& loc, std::string_view text) noexcept
        {
            if (!g_accepting_logs.load(std::memory_order_acquire)) return;
            
            g_active_producers.fetch_add(1, std::memory_order_acquire);
            if (!g_accepting_logs.load(std::memory_order_acquire))
            {
                g_active_producers.fetch_sub(1, std::memory_order_release);
                return;
            }

            if (is_verbosity_enabled(verbosity, category))
            {
                LogMessage msg{};
                msg.uptime_ns   = uptime_ns();
                msg.thread_id   = get_thread_id();
                msg.verbosity   = verbosity;
                msg.category    = &category;
                msg.location    = loc;

                const usize len = std::min(text.size(), MaxLogMessageLength - 1);
                std::memcpy(msg.text, text.data(), len);
                msg.text[len] = '\0';

                enqueue(msg);
            }

            g_active_producers.fetch_sub(1, std::memory_order_release);
        }

        [[noreturn]] void execute_fatal(const LogCategory& category, const std::source_location& loc, std::string_view text)
        {
            draco::core::logging::flush(std::chrono::milliseconds(250));

            LogMessage msg{};
            msg.uptime_ns   = uptime_ns();
            msg.thread_id   = get_thread_id();
            msg.verbosity   = LogVerbosity::Fatal;
            msg.category    = &category;
            msg.location    = loc;

            const usize len = std::min(text.size(), MaxLogMessageLength - 1);
            std::memcpy(msg.text, text.data(), len);
            msg.text[len] = '\0';

            f64 uptime_s = static_cast<f64>(msg.uptime_ns) / 1'000'000'000.0;
            std::string_view filename = msg.location.file_name();
            auto last_slash = filename.find_last_of("/\\");
            if (last_slash != std::string_view::npos) 
                filename = filename.substr(last_slash + 1);

            std::println("[FATAL] [{:.6f}] [{}] [Thread {}] [{}:{}] {}", uptime_s, category.name, msg.thread_id, filename, msg.location.line(), msg.text);

            std::shared_ptr<SinkList> current_sinks = g_sink_list.load(std::memory_order_acquire);
            if (current_sinks)
            {
                auto start = std::chrono::steady_clock::now();
                auto deadline = start + std::chrono::milliseconds(250);
                for (auto& sink : current_sinks->sinks)
                {
                    if (sink) 
                    {
                        std::unique_lock<std::mutex> io_lock(g_sink_io_mutex, std::defer_lock);
                        while (!io_lock.try_lock())
                        {
                            if (std::chrono::steady_clock::now() >= deadline)
                                break;
                            std::this_thread::yield();
                        }
                        if (io_lock.owns_lock())
                        {
                            sink->write(msg);
                            sink->flush();
                        }
                    }
                }
            }

            internal::fatal_hook();
            std::abort();
        }
    }

    LoggerMetrics get_metrics()
    {
        LoggerMetrics m{};
        m.messages_written = g_written_count.load(std::memory_order_relaxed);
        m.messages_dropped = g_dropped_count.load(std::memory_order_relaxed);
        
        usize ep = g_enqueue_pos.load(std::memory_order_relaxed);
        usize dp = g_dequeue_pos.load(std::memory_order_relaxed);
        m.queue_depth = (ep >= dp) ? (ep - dp) : 0;

        auto current = g_sink_list.load(std::memory_order_acquire);
        m.active_sinks = current ? static_cast<u32>(current->sinks.size()) : 0;
        m.uptime_ns = uptime_ns();
        return m;
    }

    void ConsoleSink::write(const LogMessage& msg)
    {
        f64 uptime_s = static_cast<f64>(msg.uptime_ns) / 1'000'000'000.0;
        std::string_view filename = msg.location.file_name();
        auto last_slash = filename.find_last_of("/\\");
        if (last_slash != std::string_view::npos) 
            filename = filename.substr(last_slash + 1);

        std::println("[{}] [{:.6f}] [{}] [Thread {}] [{}:{}] {}", verbosity_to_string(msg.verbosity), uptime_s, msg.category->name, msg.thread_id, filename, msg.location.line(), msg.text);
    }

    void ConsoleSink::flush()
    {
        std::fflush(stdout);
    }

    struct FileSink::Impl {
        std::ofstream stream;
    };

    FileSink::FileSink(const std::filesystem::path& path) : m_impl(std::make_unique<Impl>())
    {
        std::error_code ec;
        if (path.has_parent_path() && !std::filesystem::exists(path.parent_path(), ec))
        {
            std::filesystem::create_directories(path.parent_path(), ec);
            if (ec)
            {
                std::println(stderr, "[WARNING] [Logger] Failed to create directory for log file '{}': {}", path.string(), ec.message());
            }
        }
        m_impl->stream.open(path, std::ios::out | std::ios::app);
        if (!m_impl->stream.is_open())
        {
            std::println(stderr, "[WARNING] [Logger] Failed to open log file: {}", path.string());
        }
    }

    FileSink::~FileSink() = default;

    void FileSink::write(const LogMessage& msg)
    {
        if (m_impl->stream.is_open())
        {
            std::string_view filename = msg.location.file_name();
            auto last_slash = filename.find_last_of("/\\");
            if (last_slash != std::string_view::npos) 
                filename = filename.substr(last_slash + 1);

            m_impl->stream << "[" << verbosity_to_string(msg.verbosity) << "] ["  << msg.category->name << "] [Thread " << msg.thread_id << "] [" << filename << ":" << msg.location.line() << "] " << msg.text << "\n";
        }
    }

    void FileSink::flush()
    {
        if (m_impl->stream.is_open()) m_impl->stream.flush();
    }

    struct RollingFileSink::Impl {
        std::filesystem::path base_path;
        usize max_file_size;
        usize current_size{0};
        std::ofstream stream;

        void roll_file() {
            if (stream.is_open()) 
                stream.close();

            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            std::tm tm_buf{};

#if defined(_WIN32)
            localtime_s(&tm_buf, &time);
#else
            localtime_r(&time, &tm_buf);
#endif
            std::stringstream ss;
            ss << std::put_time(&tm_buf, "%Y%m%d_%H%M%S") << "_" << g_roll_seq.fetch_add(1, std::memory_order_relaxed);
            
            std::string ext = base_path.extension().string();
            std::string stem = base_path.stem().string();
            auto parent = base_path.parent_path();

            std::filesystem::path new_path = parent / (stem + "_" + ss.str() + ext);
            
            std::error_code ec;
            if (new_path.has_parent_path() && !std::filesystem::exists(new_path.parent_path(), ec))
            {
                std::filesystem::create_directories(new_path.parent_path(), ec);
                if (ec)
                {
                    std::println(stderr, "[WARNING] [Logger] Failed to create directory for rolling log file '{}': {}", new_path.string(), ec.message());
                }
            }

            stream.open(new_path, std::ios::out | std::ios::trunc);
            if (!stream.is_open())
            {
                std::println(stderr, "[WARNING] [Logger] Failed to open rolling log file: {}", new_path.string());
            }
            current_size = 0;
        }
    };

    RollingFileSink::RollingFileSink(const std::filesystem::path& base_path, usize max_file_size) : m_impl(std::make_unique<Impl>())
    {
        m_impl->base_path = base_path;
        m_impl->max_file_size = max_file_size;
        m_impl->roll_file();
    }

    RollingFileSink::~RollingFileSink() = default;

    void RollingFileSink::write(const LogMessage& msg)
    {
        if (!m_impl->stream.is_open()) 
            return;

        std::string_view filename = msg.location.file_name();
        auto last_slash = filename.find_last_of("/\\");
        if (last_slash != std::string_view::npos) 
            filename = filename.substr(last_slash + 1);

        std::stringstream ss;
        ss << "[" << verbosity_to_string(msg.verbosity) << "] [" << msg.category->name << "] [Thread " << msg.thread_id << "] [" << filename << ":" << msg.location.line() << "] " << msg.text << "\n";
        std::string str = ss.str();

        if (m_impl->current_size + str.size() > m_impl->max_file_size)
        {
            m_impl->roll_file();
        }

        m_impl->stream << str;
        m_impl->current_size += str.size();
    }

    void RollingFileSink::flush()
    {
        if (m_impl->stream.is_open()) 
            m_impl->stream.flush();
    }
}
