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
    RequestResponseHandlerBase(const std::shared_ptr<CreatorMessage> &creator_message);
    virtual std::unique_ptr<Message> processingRequestResponse(const std::unique_ptr<Message>&) = 0;
	virtual ~RequestResponseHandlerBase() = default;
protected:
    std::shared_ptr<CreatorMessage> creator_message;
    Logger& logger = Logger::getInstance();
};