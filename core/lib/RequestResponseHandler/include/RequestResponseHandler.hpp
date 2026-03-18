//
// Created by ivan on 08.03.2026.
//

#pragma once
#include <memory>

#include "../../MessageHandler/include/Message.hpp"
#include "CreatorMessage.hpp"

class IRequestResponseHandler {
public:
    IRequestResponseHandler(std::shared_ptr<CreatorMessage> creator_message) : creator_message(creator_message) {}
    virtual ~IRequestResponseHandler() = default;
    virtual std::unique_ptr<Message> processingRequestResponse(std::unique_ptr<Message>) = 0;
    std::shared_ptr<CreatorMessage> creator_message;
};
