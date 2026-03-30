//
// Created by ivan on 10.03.2026.
//
#include "ServerRequestResponseHandler.hpp"

std::unique_ptr<Message> ServerRequestResponseHandler::processingRequestResponse(std::unique_ptr<Message> message) {
    // TODO: пока я так и не понял что делать с сообшениями так пока будут отправлять статические данные

    // if (message->transaction == Transaction::Request) {
    //     logger("DEBUG") << "ServerRequestResponseHandler : message->transactionType:" << static_cast<int>(message->transaction) << "\n";
    //     json::value jv;
    //     if (message->type == "signal") {
    //         logger("DEBUG") << "ServerRequestResponseHandler : Request message->type:" << message->type << "\n";
    //         std::string json_str = R"({
    //             "type": "signal",
    //             "central_Freq": 6100,
    //             "signal": [[-8.865925598144531E1, -6.549491882324219E1]]
    //         })";
    //         jv = json::parse(json_str);
    //     } else if (message->type == "information") {
    //         logger("DEBUG") << "ServerRequestResponseHandler : Request message->type:" << message->type << "\n";
    //         std::string json_str = R"({ "numberCore": 4 })";
    //         jv = json::parse(json_str);
    //     } else {
    //         logger("WARN") << "ServerRequestResponseHandler : Request message->type:" << message->type << "\n";
    //         return nullptr;
    //     }
    //     logger("DEBUG") << "ServerRequestResponseHandler :  Create response Message"<< "\n";
    //     auto new_message = creator_message->createMessage(message->type,jv);
    //     return new_message;
    // } else if (message->transaction == Transaction::Response) {
    //     // TODO: тут мне тоже не понятно что делать
    //     logger("DEBUG") << "ServerRequestResponseHandler : message->transactionType:" << static_cast<int>(message->transaction) << "\n";
    //     json::value jv;
    //     return std::make_unique<Message>("response");
    // }
    // logger("ERROR") << "ServerRequestResponseHandler : message->transactionType:" << static_cast<int>(message->transaction) << "\n";
    // return nullptr;
}