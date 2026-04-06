#include "ClientRequestResponseHandler.hpp"

std::unique_ptr<Message> ClientRequestResponseHandler::processingRequestResponse(std::unique_ptr<Message> message) {
    // TODO: пока я так и не понял что делать с сообшениями так пока будут отправлять статические данные

    if (message->transaction == Transaction::Request) {
        logger("DEBUG") << "ServerRequestResponseHandler : message->transactionType:" << static_cast<int>(message->transaction) << "\n";
        json::value jv;
        if (message->type == "signal") {
            logger("DEBUG") << "ServerRequestResponseHandler : Request message->type:" << message->type << "\n";
            std::string json_str = R"({
                "type": "signal",
                "transaction" : "0"
            })";
            jv = json::parse(json_str);
        } else if (message->type == "information") {
            logger("DEBUG") << "ServerRequestResponseHandler : Request message->type:" << message->type << "\n";
            std::string json_str = R"({
                "type": "information",
                "transaction" : "0"
            })";
            jv = json::parse(json_str);
        } else {
            logger("WARN") << "ServerRequestResponseHandler : Request message->type:" << message->type << "\n";
            return nullptr;
        }
        logger("DEBUG") << "ServerRequestResponseHandler :  Create response Message"<< "\n";
        auto new_message = creator_message->createMessage(message->type,jv);
        return new_message;
    } else if (message->transaction == Transaction::Response) {
        return message;
    }
    logger("ERROR") << "ServerRequestResponseHandler : message->transactionType:" << static_cast<int>(message->transaction) << "\n";
    return nullptr;
}