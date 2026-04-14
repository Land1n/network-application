//
// Created by ivan on 09.04.2026.
//
#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <thread>

#include "LogEvent.hpp"

class Logger {
public:
    Logger(const std::string& name, LogLevel minLevel = LogLevel::Trace);
    void log(LogLevel level, const std::string &func_name, const std::string &message);

    void setLevel(LogLevel level);
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
private:
    std::string name_;
    std::atomic<LogLevel> minLevel_;
    std::mutex mutex_;
};

