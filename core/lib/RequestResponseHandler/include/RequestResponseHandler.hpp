//
// Created by ivan on 08.03.2026.
//

#pragma once

#include <memory>
#include "Message.hpp"
#include "CreatorMessage.hpp"
#include "Logger.hpp"
#include "Logger.hpp"

class RequestResponseHandlerBase {
public:
    RequestResponseHandlerBase(std::shared_ptr<CreatorMessage> creator_message, bool DEBUG = false);
    virtual std::unique_ptr<Message> processingRequestResponse(std::unique_ptr<Message>) = 0;
protected:
    std::shared_ptr<CreatorMessage> creator_message;
    Logger& logger = Logger::getInstance();
};