//
// Created by ivan on 11.04.2026.
//

#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>
#include <string>

#include "LogEvent.hpp"
#include "Logger.hpp"

class LoggerFactory {
public:
    static std::shared_ptr<Logger> getLogger(const std::string& name);
    static void setDefaultLevel(LogLevel level);
    static void setLoggerLevel(const std::string& name, LogLevel level);
private:
    static inline std::unordered_map<std::string, std::shared_ptr<Logger>> loggers_;
    static inline LogLevel defaultLevel_ = LogLevel::Trace;
    static inline std::mutex mutex_;
};