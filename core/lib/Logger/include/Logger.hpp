//
// Created by ivan on 09.04.2026.
//
#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "LogEvent.hpp"

class Logger {
public:
    static Logger& getInstance();

    void log(LogLevel level, const std::string &func_name, const std::string &message);
    void setLevel(LogLevel level);
private:
    Logger(std::string name, LogLevel minLevel);
    std::string name_;
    std::atomic<LogLevel> minLevel_;
    std::mutex mutex_;
};

