//
// Created by ivan on 08.03.2026.
//
#pragma once

#include <memory>
#include "message/transport_message.h"
#include "message/message.h"
#include "message_handler/creator_message.h"
#include "logger/logger.h"

class MessageHandler {
public:
	MessageHandler();

	std::unique_ptr<Message> parse(const TransportMessage& transport_message);
	TransportMessage serialize(const std::unique_ptr<Message>& message);

	std::shared_ptr<CreatorMessage> creator_message = std::make_shared<CreatorMessage>();

private:
	Logger& logger = Logger::getInstance();
};