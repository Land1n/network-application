#pragma once

#include <chrono>
#include <thread>
#include <mutex>
#include <sstream>

enum LogLevel {
    Trace = 6,
    Debug = 5,
    Critical = 4,
    Error = 3,
    Warn = 2,
    Info = 1,
    Tests = 0,
    NoLog = -1
};

class LogEvent {
public:
    LogEvent(LogLevel lvl, const std::string &name_logger, const std::string &func, const std::string &msg);
    std::string formatEvent() const;

    friend std::ostream& operator<<(std::ostream& os, const LogEvent& l_e);

private:
    static std::string levelToString(LogLevel level);
    static std::string levelToColor(LogLevel level);

    LogLevel level;
    std::string name_logger;
    std::string message;
    std::string function;
    std::chrono::system_clock::time_point time;
    std::thread::id threadId;
};