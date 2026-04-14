#include "LogEvent.hpp"
#include <iomanip>
#include <boost/format.hpp>

static std::string formatTime(const std::chrono::system_clock::time_point& tp) {
    auto tt = std::chrono::system_clock::to_time_t(tp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  tp.time_since_epoch()) % 1000;
    std::tm tm_buf;
    localtime_r(&tt, &tm_buf);
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

LogEvent::LogEvent(LogLevel lvl, const std::string &name_logger,
                   const std::string &func, const std::string &msg)
    : level(lvl), name_logger(name_logger), function(func),
      time(std::chrono::system_clock::now()), threadId(std::this_thread::get_id()),
      message(msg) {}

std::string LogEvent::formatEvent() const {
    const std::string resetColor = "\033[0m ";
    std::string color = levelToColor(level);
    return (boost::format("%s[%s] [%-8s] [%5d] [%-20s] [%-21s] %s%s")
            % color
            % formatTime(time)
            % levelToString(level)
            % threadId
            % name_logger
            % function
            % message
            % resetColor).str();
}

std::ostream& operator<<(std::ostream& os, const LogEvent& l_e) {
    os << l_e.formatEvent();
    return os;
}

std::string LogEvent::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Critical: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

std::string LogEvent::levelToColor(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "\033[36m";
        case LogLevel::Debug: return "\033[90m";
        case LogLevel::Info:  return "\033[32m";
        case LogLevel::Warn:  return "\033[33m";
        case LogLevel::Error: return "\033[31m";
        case LogLevel::Critical: return "\033[35m";
        default: return "\033[0m";
    }
}