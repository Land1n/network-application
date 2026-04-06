//
// Created by ivan on 10.03.2026.
//
#include "ServerRequestResponseHandler.hpp"

std::unique_ptr<Message> ServerRequestResponseHandler::processingRequestResponse(std::unique_ptr<Message> message) {
    // TODO: пока я так и не понял что делать с сообшениями так пока будут отправлять статические данные
    logger("DEBUG") << "ServerRequestResponseHandler [processingRequestResponse()] : message->transaction = " <<
            static_cast<int>(message->transaction) << "\n";
    logger("DEBUG") << "ServerRequestResponseHandler [processingRequestResponse()] : message->type = " << message->type
            << "\n";

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
        logger("DEBUG") << "ServerRequestResponseHandler [processingRequestResponse()] :  Create response Message" <<
                "\n";
        auto new_message = creator_message->createMessage(message->type, Transaction::Response, jv);
        return new_message;
    } else if (message->transaction == Transaction::Response) {
        return message;
    }
    return nullptr;
}
