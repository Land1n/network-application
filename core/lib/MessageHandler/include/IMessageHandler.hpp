//
// Created by ivan on 08.03.2026.
//

#pragma once

#include <memory>

#include "TransportMessage.hpp"
#include "Message.hpp"


class IMessageHandler {
public:
    virtual ~IMessageHandler() = default;

    virtual std::unique_ptr<Message> parse(const TransportMessage &transport_message) = 0;
    virtual TransportMessage serialize(std::unique_ptr<Message> message) = 0;
};