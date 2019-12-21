#pragma once
#include <fstream> // std::ofstream
#include <string> // std::string
#include <stdexcept> // std::runtime_error
#include <chrono>
#include <mutex>
#if defined(LOG_ERROR) || defined (LOG_WARNING) || defined(LOG_DEBUG) || defined(SET_LOG_LEVEL_ERROR) || defined(SET_LOG_LEVEL_WARNING) || defined(SET_LOG_LEVEL_DEBUG)
static_assert(false, "One or multiple logging macros were already defined in Logger.cpp");
#endif
#if defined(CONCAT_IMPL) || defined(MACRO_CONCAT)
static_assert(false, "CONCAT_IMPL or MACRO_CONCAT already defined in Logger.cpp");
#endif
#define CONCAT_IMPL(x, y) x##y
#define MACRO_CONCAT(x, y) CONCAT_IMPL(x, y)
#define LOG_ERROR	if (logging::Logger::getLogger().getLogLevel() >= logging::LogLevel::Error) logging::Logger::getLogger().log()
#define LOG_WARNING	if (logging::Logger::getLogger().getLogLevel() >= logging::LogLevel::Warning) logging::Logger::getLogger().log()
#define LOG_DEBUG	if (logging::Logger::getLogger().getLogLevel() >= logging::LogLevel::Debug) logging::Logger::getLogger().log()
#define SET_LOG_LEVEL_ERROR		logging::Logger::getLogger().setLogLevel(logging::LogLevel::Error)
#define SET_LOG_LEVEL_WARNING	logging::Logger::getLogger().setLogLevel(logging::LogLevel::Warning)
#define SET_LOG_LEVEL_DEBUG		logging::Logger::getLogger().setLogLevel(logging::LogLevel::Debug)
#define LOG_SCOPE				logging::LogScope MACRO_CONCAT(logScope_AbCTNHTNEhaANeitnTTOmmoniaet_a, __COUNTER__){ __func__ }

namespace logging {
    class LoggerException : public std::runtime_error {
    public:
        explicit LoggerException(std::string msg);

        virtual char const *what() const noexcept override;
    }; // END of class LoggerException


    namespace detail {
        class ThreadSafeOfStreamWrapper final {
        public:
            using this_type = ThreadSafeOfStreamWrapper;
            using Mutex = std::mutex;
            using Lock = std::lock_guard<Mutex>;

            template <class ...Args>
            explicit ThreadSafeOfStreamWrapper(Args &&...args)
                : ofstream_{ std::forward<Args>(args)... } {
                
                if (!ofstream_) {
                    throw LoggerException{ "could not open file_ for logging." };
                }
            }

            template <class Type>
            friend this_type &operator<<(this_type &stream, Type &&type) {
                Lock lock{ stream.mutex_ };
                stream.ofstream_ << std::forward<Type>(type);
                return stream;
            }

            ~ThreadSafeOfStreamWrapper() {
                ofstream_ << std::flush;
            }

        private:
            Mutex mutex_;
            std::ofstream ofstream_;
        }; // END of class ThreadSafeOfStreamWrapper
    } // END of namespace detail

    enum class LogLevel {
        Error,
        Warning,
        Debug,
    }; // END of enum class LogLevel

    class Logger final {
    public:
        using this_type = Logger;
        using value_type = detail::ThreadSafeOfStreamWrapper;
        static std::string const file_;
        static std::ios::openmode const openMode_;
        static std::chrono::time_point<std::chrono::steady_clock> const baseTimestamp_;

        Logger(this_type const &) = delete;
        Logger(this_type &&) = delete;
        this_type &operator=(this_type const &) = delete;
        this_type &operator=(this_type &&) = delete;

        static this_type &getLogger();

        value_type &log();

        void setLogLevel(LogLevel logLevel);

        LogLevel getLogLevel() const;

    private:
        Logger();

        LogLevel level_;
        value_type logfile_;
    }; // END of class Logger

    class LogScope final {
    public:
        using this_type = LogScope;
        using value_type = std::string;

        explicit LogScope(value_type val) noexcept;

        ~LogScope();

    private:
        value_type func_;
        Logger &logger_;
    }; // END of class LogScope

} // END of namespace logging
