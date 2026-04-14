#include "ClientRequestResponseHandler.hpp"
#include <boost/json.hpp>

namespace json = boost::json;

std::unique_ptr<Message> ClientRequestResponseHandler::processingRequestResponse(std::unique_ptr<Message> message) {
    if (message->transaction == Transaction::Request) {
        logger->log(LogLevel::Debug, __func__,
                    "message->transaction = " + std::to_string(static_cast<int>(message->transaction)));
        json::value jv;
        if (message->type == "signal") {
            logger->log(LogLevel::Debug, __func__, "Request message->type: " + message->type);
            std::string json_str = R"({
                "type": "signal",
                "transaction" : 0
            })";
            jv = json::parse(json_str);
        } else if (message->type == "information") {
            logger->log(LogLevel::Debug, __func__, "Request message->type: " + message->type);
            std::string json_str = R"({
                "type": "information",
                "transaction" : 0
            })";
            jv = json::parse(json_str);
        } else {
            logger->log(LogLevel::Warn, __func__, "Request message->type: " + message->type);
            return nullptr;
        }
        logger->log(LogLevel::Debug, __func__, "Create response Message");
        // Исправлено: добавлен Transaction::Request (или какой нужен)
        auto new_message = creator_message->createMessage(message->type, Transaction::Request, jv);
        return new_message;
    } else if (message->transaction == Transaction::Response) {
        return message;
    }
    logger->log(LogLevel::Error, __func__,
                "Unexpected transaction: " + std::to_string(static_cast<int>(message->transaction)));
    return nullptr;
}