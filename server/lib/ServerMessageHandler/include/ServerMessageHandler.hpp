//
// Created by ivan on 10.03.2026.
//

#pragma once

#include "IMessageHandler.hpp"
#include "TransportMessage.hpp"
#include "Message.hpp"

class ServerMessageHandler : public IMessageHandler {
public:
    std::unique_ptr<Message> parse(const TransportMessage &transport_message) override;
    TransportMessage serialize(std::unique_ptr<Message> message) override;
};