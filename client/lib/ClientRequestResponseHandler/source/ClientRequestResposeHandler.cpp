#include <iostream>

#include "ClientRequestResponseHandler.hpp"
#include "RawMessage.hpp"
#include <boost/json.hpp>

#include "LogEvent.hpp"
#include "../../../../core/lib/TransportHandler/include/TransportHandler/Transaction.hpp"

namespace json = boost::json;

ClientRequestResponseHandler::ClientRequestResponseHandler(const std::shared_ptr<CreatorMessage>& creator_message) :
	RequestResponseHandlerBase(creator_message)
{}

std::unique_ptr<Message> ClientRequestResponseHandler::processingRequestResponse(const std::unique_ptr<Message>& message) {
    logger.log(LogLevel::Info, __func__,"Message type: " + message->type);
    logger.log(LogLevel::Info, __func__,"Processing new message...");
    if (message->transaction == Transaction::Request) {
        logger.log(LogLevel::Debug, __func__,
                    "message->transaction = " + std::to_string(static_cast<int>(message->transaction)));
        json::value jv;
        logger.log(LogLevel::Debug, __func__, "Request message->type: " + message->type);
        if (message->type == "signal") {
            std::string json_str = R"({
                "type": "signal",
                "transaction" : 0
            })";
            jv = json::parse(json_str);
        } else if (message->type == "information") {
            std::string json_str = R"({
                "type": "information",
                "transaction" : 0
            })";
            jv = json::parse(json_str);
        } else if (message->type == "raw") {
            std::string json_str = R"({
                "type": "raw",
                "transaction" : 0
            })";
            jv = json::parse(json_str);
        } else {
            return nullptr;
        }
        logger.log(LogLevel::Debug, __func__, "Create response Message");
        auto new_message = creator_message->createMessage(message->type, Transaction::Request, jv);
        return new_message;
    } else if (message->transaction == Transaction::Response) {
        return std::make_unique<Message>(message->type,message->transaction);
    }
    logger.log(LogLevel::Error, __func__,
                "Unexpected transaction: " + std::to_string(static_cast<int>(message->transaction)));
    return nullptr;
}