#include "Logger.h"
#include <utility> // std::move
#include <mutex>

namespace logging {
    LoggerException::LoggerException(std::string msg)
        : std::runtime_error{ std::move(msg) } { }

    char const *LoggerException::what() const noexcept {
        return std::runtime_error::what();
    }

    Logger::this_type &Logger::getLogger() {
        static this_type instance{ };
        return instance;
    }

    Logger::value_type &Logger::log() {
        auto curTimestamp = std::chrono::steady_clock::now() - baseTimestamp_;
        logfile_ << std::chrono::duration_cast<std::chrono::microseconds>(curTimestamp).count() << "us: ";
        return logfile_;
    }

    void Logger::setLogLevel(LogLevel logLevel) {
        level_ = logLevel;
    }

    LogLevel Logger::getLogLevel() const {
        return level_;
    }

    Logger::Logger() : level_{ LogLevel::Debug }, logfile_{ file_, openMode_ } { }

    std::string const Logger::file_ = __DATE__ " LogFile.txt";
    std::ios::openmode const Logger::openMode_ = std::ios::trunc;
    std::chrono::time_point<std::chrono::steady_clock> const Logger::baseTimestamp_ = std::chrono::steady_clock::now();

    LogScope::LogScope(value_type val) noexcept : func_{ std::move(val) },
        logger_{ Logger::getLogger() } {

        LOG_DEBUG << "Entering function " << func_ << '\n';
    }

    LogScope::~LogScope() {
        LOG_DEBUG << "Exiting function " << func_ << '\n';
    }

} // END of namespace logging
