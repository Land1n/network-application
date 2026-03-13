//
// Created by ivan on 08.03.2026.
//

#pragma once

#include <memory>

#include "TransportMessage.hpp"
#include "Message.hpp"
#include "CreatorMessage.hpp"
#include "../../Message/include/CreatorMessage.hpp"

class IMessageHandler {
public:
    virtual ~IMessageHandler() = default;

    virtual std::unique_ptr<Message> parse(const TransportMessage &transport_message) = 0;
    virtual TransportMessage serialize(std::unique_ptr<Message> message) = 0;

    std::shared_ptr<CreatorMessage> creator_message = std::make_shared<CreatorMessage>();
};
