#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <string_view>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <memory>

import core.logging;
import core.stdtypes;

inline const draco::core::logging::LogCategory LogTest{"Test"};
inline const draco::core::logging::LogCategory LogRendering{"Rendering"};
inline const draco::core::logging::LogCategory LogPhysics{"Physics"};

namespace draco::core::logging
{
    namespace
    {
        struct TestSink final : ILogSink
        {
            std::atomic<draco::u32> write_count{0};
            std::atomic<draco::u32> flush_count{0};
            std::mutex last_mutex;
            LogMessage last{};
            draco::u64 expected_seq{0};

            void write(const LogMessage& msg) override
            {
                std::lock_guard lock(last_mutex);
                last = msg;
                write_count.fetch_add(1);
                
                if (msg.verbosity != LogVerbosity::Fatal)
                {
                    REQUIRE(msg.sequence_id == expected_seq);
                    expected_seq++;
                }
            }

            void flush() override
            {
                flush_count.fetch_add(1);
            }
        };

        void wait_for_condition(auto&& fn, int timeout_ms = 2000)
        {
            using namespace std::chrono;
            auto start = steady_clock::now();
            while (!fn())
            {
                if (duration_cast<milliseconds>(steady_clock::now() - start).count() > timeout_ms)
                    break;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }
}

TEST_CASE("Logger init and shutdown do not crash")
{
    using namespace draco::core::logging;
    init();
    shutdown();
}

TEST_CASE("Logger dispatches messages to sinks (async)")
{
    using namespace draco::core::logging;
    init();

    auto sink = std::make_shared<TestSink>();
    add_sink(sink);

    log(LogVerbosity::Info, LogTest, "Hello Logging");

    wait_for_condition([&] { return sink->write_count.load() >= 1; });

    REQUIRE(sink->write_count.load() == 1);
    REQUIRE(sink->last.category == &LogTest);
    REQUIRE(sink->last.verbosity == LogVerbosity::Info);
    REQUIRE(std::string_view(sink->last.text) == "Hello Logging");

    shutdown();
}

TEST_CASE("Logger flush calls sink flush and acts as a safe barrier")
{
    using namespace draco::core::logging;
    init();

    auto sink = std::make_shared<TestSink>();
    add_sink(sink);

    log(LogVerbosity::Info, LogTest, "Barrier Test");
    flush();

    REQUIRE(sink->write_count.load() == 1);
    REQUIRE(sink->flush_count.load() == 1);

    shutdown();
}

TEST_CASE("Logger flush with explicit timeout does not hang")
{
    using namespace draco::core::logging;
    init();

    auto sink = std::make_shared<TestSink>();
    add_sink(sink);

    log(LogVerbosity::Info, LogTest, "Timed Flush");
    flush(std::chrono::milliseconds(250));

    REQUIRE(sink->write_count.load() == 1);
    REQUIRE(sink->flush_count.load() == 1);

    shutdown();
}

TEST_CASE("Logger generates monotonically increasing sequence ids")
{
    using namespace draco::core::logging;
    init();

    auto sink = std::make_shared<TestSink>();
    add_sink(sink);

    log(LogVerbosity::Info, LogTest, "A");
    log(LogVerbosity::Info, LogTest, "B");
    log(LogVerbosity::Info, LogTest, "C");

    flush();

    REQUIRE(sink->write_count.load() == 3);
    REQUIRE(sink->last.sequence_id == 2);

    shutdown();
}

TEST_CASE("Logger respects global verbosity filter")
{
    using namespace draco::core::logging;
    init();

    auto sink = std::make_shared<TestSink>();
    add_sink(sink);

    set_global_verbosity(LogVerbosity::Warning);

    log(LogVerbosity::Info, LogTest, "Hidden");
    log(LogVerbosity::Error, LogTest, "Visible");

    flush();

    REQUIRE(sink->write_count.load() == 1);

    shutdown();
}

TEST_CASE("Logger handles multiple sinks and removal")
{
    using namespace draco::core::logging;
    init();

    auto a = std::make_shared<TestSink>();
    auto b = std::make_shared<TestSink>();

    add_sink(a);
    add_sink(b);

    log(LogVerbosity::Info, LogTest, "Multi Sink");
    flush();

    REQUIRE(a->write_count.load() == 1);
    REQUIRE(b->write_count.load() == 1);

    remove_sink(b);
    log(LogVerbosity::Info, LogTest, "Only A");
    flush();

    REQUIRE(a->write_count.load() == 2);
    REQUIRE(b->write_count.load() == 1);

    shutdown();
}

TEST_CASE("Async System Stress Test - Dropping Allowed")
{
    using namespace draco::core::logging;
    init();

    auto sink = std::make_shared<TestSink>();
    add_sink(sink);

    constexpr int NUM_THREADS = 16;
    constexpr int MSG_PER_THREAD = 100000; 

    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        threads.emplace_back([i]() {
            for (int j = 0; j < MSG_PER_THREAD; ++j)
            {
                log(LogVerbosity::Info, LogTest, "Stress Test Message");
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    flush();

    LoggerMetrics m = get_metrics();
    REQUIRE(m.messages_written + m.messages_dropped == NUM_THREADS * MSG_PER_THREAD);
    REQUIRE(sink->write_count.load() == m.messages_written);

    shutdown(); 
}

TEST_CASE("Logger handles Queue Overflow gracefully")
{
    using namespace draco::core::logging;
    init();

    auto sink = std::make_shared<TestSink>();
    add_sink(sink);

    for (int i = 0; i < 50000; ++i)
    {
        log(LogVerbosity::Info, LogTest, "Spam");
    }

    flush();

    LoggerMetrics m = get_metrics();
    REQUIRE(m.messages_written + m.messages_dropped == 50000);

    shutdown();
}

struct FatalTestException {};

TEST_CASE("Fatal path writes directly and aborts")
{
    using namespace draco::core::logging;
    init();

    auto sink = std::make_shared<TestSink>();
    add_sink(sink);

    auto previous_fatal_hook = draco::core::logging::internal::fatal_hook;

    struct FatalHookRestore final {
        void (**slot)();
        void (*old_value)();
        ~FatalHookRestore() { *slot = old_value; }
    } fatal_hook_restore{&draco::core::logging::internal::fatal_hook, previous_fatal_hook};

    draco::core::logging::internal::fatal_hook = [] { throw FatalTestException{}; };

    bool caught = false;
    try
    {
        fatal(LogTest, "Fatal crash {}!", 123);
    }
    catch (const FatalTestException&)
    {
        caught = true;
    }

    CHECK(caught);
    CHECK(sink->write_count.load() == 1);
    CHECK(sink->flush_count.load() == 2);
    CHECK(std::string_view(sink->last.text) == "Fatal crash 123!");

    shutdown();
}

TEST_CASE("Logging drops silently after shutdown")
{
    using namespace draco::core::logging;
    init();

    auto sink = std::make_shared<TestSink>();
    add_sink(sink);

    shutdown();

    log(LogVerbosity::Info, LogTest, "Post-shutdown ignored");

    REQUIRE(sink->write_count.load() == 0);
}

TEST_CASE("File Sinks Validation")
{
    using namespace draco::core::logging;
    init();

    std::filesystem::path test_path = "test_output.log";
    std::filesystem::remove(test_path);

    {
        auto file_sink = std::make_shared<FileSink>(test_path);
        add_sink(file_sink);

        log(LogVerbosity::Error, LogTest, "File Sink Data");
        flush();
        remove_sink(file_sink);
    }

    REQUIRE(std::filesystem::exists(test_path));
    std::ifstream infile(test_path);
    std::string line;
    std::getline(infile, line);
    REQUIRE(line.find("File Sink Data") != std::string::npos);

    std::filesystem::remove(test_path);
    shutdown();
}

TEST_CASE("Formatting and Macro Validation")
{
    using namespace draco::core::logging;
    init();

    auto sink = std::make_shared<TestSink>();
    add_sink(sink);

    log(LogVerbosity::Info, LogTest, "Player {} joined", 42);
    flush();
    REQUIRE(std::string_view(sink->last.text) == "Player 42 joined");

    log(LogVerbosity::Warning, LogTest, "FPS dropped to {}", 59.5);
    flush();
    REQUIRE(std::string_view(sink->last.text) == "FPS dropped to 59.5");

    shutdown();
}

TEST_CASE("Category Overrides Validation")
{
    using namespace draco::core::logging;
    init();

    auto sink = std::make_shared<TestSink>();
    add_sink(sink);

    set_global_verbosity(LogVerbosity::Info);
    LogRendering.override_verbosity.store(static_cast<draco::i32>(LogVerbosity::Verbose), std::memory_order_relaxed);
    LogPhysics.override_verbosity.store(static_cast<draco::i32>(LogVerbosity::Error), std::memory_order_relaxed);

    log(LogVerbosity::Verbose, LogRendering, "Render Pass");
    log(LogVerbosity::Info, LogPhysics, "Physics Update");

    flush();

    REQUIRE(sink->write_count.load() == 1);
    REQUIRE(std::string_view(sink->last.text) == "Render Pass");

    shutdown();
}

TEST_CASE("Performance Metrics Operations")
{
    using namespace draco::core::logging;
    init();

    auto sink = std::make_shared<TestSink>();
    add_sink(sink);

    log(LogVerbosity::Info, LogTest, "Metrics Check");
    flush();

    LoggerMetrics m = get_metrics();
    REQUIRE(m.messages_written == 1);
    REQUIRE(m.active_sinks == 1);
    REQUIRE(m.uptime_ns > 0);

    shutdown();
}
