//
// Created by ivan on 14.03.2026.
//
#include "Client.hpp"

#include <iostream>

#include "TransportHandler.hpp"
#include "ConnectionHandler.hpp"
#include "MessageHandler.hpp"

#include "SignalMessage.hpp"
#include "InformationMessage.hpp"
#include "../lib/ClientRequestResponseHandler/include/ClientRequestResponseHandler.hpp"

#include "sdrlogger/sdrlogger.h"

Client::Client(std::string address, int port) : address(address), port(port) {
    connection_handler = std::make_shared<ConnectionHandler>(
        this->address, this->port, ConnectionHandlerType::Client
    );
    start();
}

void Client::stop() {
    connection_handler->stop();
}

Client::~Client() {
    stop();
}

void Client::start() {
    connection_handler->setTaskSocket([](std::shared_ptr<tcp::socket> sock) {
        auto &logger = BaseLogger::get();
        TransportHandler transport_handler(sock);
        MessageHandler message_handler;
        int i;
        std::cout << "Type message: ";
        std::cin >> i;
        auto transport_message = message_handler.serialize(
            std::make_unique<Message>((i == 1 ? "signal" : "information"), Transaction::Request)
        );
        transport_handler.send(transport_message);

        TransportMessage new_transport_message = transport_handler.read();
        if (new_transport_message.type != "" && new_transport_message.type != "error") {
            auto new_message = message_handler.parse(new_transport_message);

            if (new_transport_message.type == "signal") {
                auto *new_message_signal = dynamic_cast<SignalMessage *>(new_message.get());
                if (new_message_signal != nullptr) {
                    logger("INFO") << "Data from message: ";
                    for (const auto &sample: new_message_signal->getSignal()) {
                        std::cout << sample << " ";
                    }
                    std::cout << std::endl;
                }
            } else if (new_transport_message.type == "information") {
                auto *new_message_information = dynamic_cast<InformationMessage *>(new_message.get());
                logger("INFO") << "Data from message: " << new_message_information->getNumberCore();
            } else {
                logger("ERROR") << "Unknown message type: " << new_transport_message.type;
            }
        }

    });

    auto socket = connection_handler->connect();

}
