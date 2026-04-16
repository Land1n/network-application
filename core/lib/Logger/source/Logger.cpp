//
// Created by ivan on 09.04.2026.
//

#include "Logger.hpp"

#include <iostream>

Logger::Logger(std::string name, LogLevel minLevel)
    : name_(std::move(name)), minLevel_(minLevel) {}

void Logger::log(LogLevel level, const std::string& func_name, const std::string& message) {
    if (static_cast<int>(level) >= static_cast<int>(minLevel_.load())) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << LogEvent(level, name_, func_name, message) << std::endl;
    }
}

void Logger::setLevel(LogLevel level) {
    minLevel_.store(level);
}

Logger& Logger::getInstance() {
    static Logger instance("GlobalLogger", LogLevel::Trace);
    return instance;
}
