#include "ServerRequestResponseHandler.hpp"
#include <boost/json.hpp>

namespace json = boost::json;

std::unique_ptr<Message> ServerRequestResponseHandler::processingRequestResponse(std::unique_ptr<Message> message) {
    logger.log(LogLevel::Debug, __func__,
                "message->transaction = " + std::to_string(static_cast<int>(message->transaction)));
    logger.log(LogLevel::Debug, __func__, "message->type = " + message->type);

    if (message->transaction == Transaction::Request) {
        std::string json_str;
        json::value jv;

        if (message->type == "signal") {
            json_str = R"({
                "type": "signal",
                "transaction" : 1,
                "central_Freq": 6100,
                "signal": [[-8.865925598144531E1, -6.549491882324219E1]]
            })";
        } else if (message->type == "information") {
            json_str = R"({
                "type": "information",
                "transaction" : 1,
                "numberCore": 4
            })";
        } else {
            json_str = R"({
                "type": "unknown",
                "transaction" : -1
            })";
        }
        jv = json::parse(json_str);
        logger.log(LogLevel::Debug, __func__, "Create response Message");
        auto new_message = creator_message->createMessage(message->type, Transaction::Response, jv);
        return new_message;
    } else if (message->transaction == Transaction::Response) {
        return message;
    }
    return nullptr;
}