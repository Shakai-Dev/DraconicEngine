module;

#include <atomic>
#include <memory>
#include <string_view>
#include <format>
#include <chrono>
#include <source_location>
#include <filesystem>
#include <type_traits>
#include <concepts>
#include <utility>

export module core.logging;

import core.stdtypes;

export namespace draco::core::logging
{
    inline constexpr usize MaxLogMessageLength = 512;

    enum class LogVerbosity : u8
    {
        Fatal,
        Error,
        Warning,
        Display,
        Info,
        Verbose,
        VeryVerbose
    };

    inline constexpr LogVerbosity CompileTimeVerbosityLimit = LogVerbosity::VeryVerbose;

    struct LogCategory
    {
        const char* name;
        mutable std::atomic<i32> override_verbosity{ -1 };
    };

    struct LogMessage
    {
        u64 sequence_id;
        u64 uptime_ns;
        u32 thread_id;

        LogVerbosity verbosity;
        const LogCategory* category;
        
        std::source_location location;

        char text[MaxLogMessageLength];
    };

    struct ILogSink
    {
        virtual ~ILogSink() = default;
        virtual void write(const LogMessage& msg) = 0;
        virtual void flush() = 0;
    };

    class ConsoleSink final : public ILogSink
    {
    public:
        void write(const LogMessage& msg) override;
        void flush() override;
    };

    class FileSink final : public ILogSink
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    public:
        FileSink(const std::filesystem::path& path);
        ~FileSink() override;
        
        void write(const LogMessage& msg) override;
        void flush() override;
    };

    class RollingFileSink final : public ILogSink
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    public:
        RollingFileSink(const std::filesystem::path& base_path, usize max_file_size);
        ~RollingFileSink() override;

        void write(const LogMessage& msg) override;
        void flush() override;
    };

    struct LoggerMetrics
    {
        u64 messages_written;
        u64 messages_dropped;
        u64 queue_depth;
        u32 active_sinks;
        u64 uptime_ns;
    };

    namespace internal
    {
        extern void (*fatal_hook)();
        bool is_verbosity_enabled(LogVerbosity verbosity, const LogCategory& category) noexcept;
        void enqueue_log(LogVerbosity verbosity, const LogCategory& category, const std::source_location& loc, std::string_view text) noexcept;
        [[noreturn]] void execute_fatal(const LogCategory& category, const std::source_location& loc, std::string_view text);
    }

    void init();
    void shutdown();
    void flush(std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
    
    void set_global_verbosity(LogVerbosity verbosity);
    
    void add_sink(std::shared_ptr<ILogSink> sink);
    void remove_sink(std::shared_ptr<ILogSink> sink);

    LoggerMetrics get_metrics();

    template <typename... Args>
    struct LogFormatString 
    {
        std::format_string<Args...> fmt;
        std::source_location loc;

        template <typename T>
            requires std::constructible_from<std::format_string<Args...>, T>
        
        consteval LogFormatString(const T& s, std::source_location l = std::source_location::current()) : fmt(s), loc(l) {}
    };

    template<typename... Args>
    void log(LogVerbosity verbosity, const LogCategory& category, LogFormatString<std::type_identity_t<Args>...> fmt, Args&&... args) noexcept
    {
        if constexpr (CompileTimeVerbosityLimit < LogVerbosity::VeryVerbose)
        {
            if (verbosity > CompileTimeVerbosityLimit)
                return;
        }

        if (!internal::is_verbosity_enabled(verbosity, category))
            return;

        char buffer[MaxLogMessageLength];
        auto result = std::format_to_n(buffer, MaxLogMessageLength - 1, fmt.fmt, std::forward<Args>(args)...);
        *result.out = '\0';
        internal::enqueue_log(verbosity, category, fmt.loc, buffer);
    }

    template<typename... Args>
    [[noreturn]] void fatal(const LogCategory& category, LogFormatString<std::type_identity_t<Args>...> fmt, Args&&... args)
    {
        char buffer[MaxLogMessageLength];
        auto result = std::format_to_n(buffer, MaxLogMessageLength - 1, fmt.fmt, std::forward<Args>(args)...);
        *result.out = '\0';
        internal::execute_fatal(category, fmt.loc, buffer);
    }
}
