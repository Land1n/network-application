//
// Created by ivan on 08.03.2026.
//

#pragma once

#include <memory>

#include "TransportMessage.hpp"
#include "Message.hpp"
#include "CreatorMessage.hpp"
#include "sdrlogger/sdrlogger.h"

class MessageHandler {
public:
    MessageHandler(bool DEBUG = false);

    std::unique_ptr<Message> parse(const TransportMessage &transport_message);
    TransportMessage serialize(std::unique_ptr<Message> message);

    std::shared_ptr<CreatorMessage> creator_message = std::make_shared<CreatorMessage>();
    BaseLogger& logger = BaseLogger::get();
};
