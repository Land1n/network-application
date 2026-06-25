//
// Created by ivan on 10.03.2026.
//
#include "message_handler/message_handler.h"
#include "message/signal_message.h"
#include "message/information_message.h"
#include "message/raw_message.h"
#include "utils/error_handler.h" // из Utils
#include <boost/json.hpp>

namespace json = boost::json;

MessageHandler::MessageHandler()
{
	creator_message->addMessageOnMap("signal", [](const std::string& type, Transaction transaction, json::value& jv) {
		return std::make_unique<SignalMessage>(type, transaction, jv);
	});
	creator_message->addMessageOnMap("information",
	                                 [](const std::string& type, Transaction transaction, json::value& jv) {
		                                 return std::make_unique<InformationMessage>(type, transaction, jv);
	                                 });
	creator_message->addMessageOnMap("raw", [](const std::string& type, Transaction transaction, json::value& jv) {
		return std::make_unique<RawMessage>(type, transaction, jv);
	});
}

std::unique_ptr<Message> MessageHandler::parse(const TransportMessage& transport_message)
{
	const std::string sv(transport_message.payload.begin(), transport_message.payload.end());
	json::value jv;
	error_code error;
	jv = boost::json::parse(sv, error);
	ErrorHandler::check_error(error, "MessageHandler::parse");
	if(error)
		return nullptr;
	auto message = creator_message->createMessage(transport_message.type, transport_message.transaction, jv);
	return message;
}

TransportMessage MessageHandler::serialize(const std::unique_ptr<Message>& message)
{
	if(!message)
		return {};
	std::string json_str = json::serialize(message->serialize());
	std::vector<uint8_t> bytes(json_str.begin(), json_str.end());
	return {message->type, message->transaction, bytes};
}