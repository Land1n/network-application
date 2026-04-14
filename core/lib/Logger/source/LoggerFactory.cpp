//
// Created by ivan on 11.04.2026.
//
#include "LoggerFactory.hpp"

#include <memory>
#include <mutex>


std::shared_ptr<Logger> LoggerFactory::getLogger(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = loggers_.find(name);
    if (it != loggers_.end()) {
        return it->second;
    }
    auto logger = std::make_shared<Logger>(name, defaultLevel_);
    loggers_[name] = logger;
    return logger;
}

void LoggerFactory::setDefaultLevel(LogLevel level) {
    defaultLevel_ = level;
};

void LoggerFactory::setLoggerLevel(const std::string& name, LogLevel level) {
    auto logger = getLogger(name);
    logger->setLevel(level);
}