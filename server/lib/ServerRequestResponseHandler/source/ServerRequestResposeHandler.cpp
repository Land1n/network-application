//
// Created by ivan on 10.03.2026.
//
#include "ServerRequestResponseHandler.hpp"

std::unique_ptr<Message> ServerRequestResponseHandler::processingRequestResponse(std::unique_ptr<Message> message) {
    // TODO: пока я так и не понял что делать с сообшениями так пока будут отправлять статические данные
    if (message->transactionType == TransactionType::Request) {
        json::value jv;
        if (message->type == "signal") {
            std::string json_str = R"({
                "type": "signal",
                "central_Freq": 6100,
                "signal": [[-8.865925598144531E1, -6.549491882324219E1]]
            })";
            jv = json::parse(json_str);
        } else if (message->type == "information") {
            std::string json_str = R"({ "numberCore": 4 })";
            jv = json::parse(json_str);
        } else {
            return nullptr;
        }
        auto new_message = creator_message->createMessage(message->type,jv);
        new_message->setTransactionType();
        return new_message;
    } else if (message->transactionType == TransactionType::Response) {
        // TODO: тут мне тоже не понятно что делать
        json::value jv;
        return std::make_unique<Message>("response");
    }
    return nullptr;
};