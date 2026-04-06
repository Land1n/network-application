//
// Created by ivan on 08.03.2026.
//

#pragma once
#include <memory>

#include "Message.hpp"

#include "CreatorMessage.hpp"

#include "sdrlogger/sdrlogger.h"

class RequestResponseHandlerBase {
public:
    RequestResponseHandlerBase(std::shared_ptr<CreatorMessage> creator_message, bool DEBUG = false);
    virtual std::unique_ptr<Message> processingRequestResponse(std::unique_ptr<Message>) = 0;
    std::shared_ptr<CreatorMessage> creator_message;
    BaseLogger& logger = BaseLogger::get();
};
