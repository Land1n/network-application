//
// Created by ivan on 09.03.2026.
//

#include <string>

#include "Server.hpp"

#include <iostream>

#include "ConnectionHandler.hpp"
#include "TransportHandler.hpp"
#include "MessageHandler.hpp"
#include "ServerRequestResponseHandler.hpp"
#include "sdrlogger/sdrlogger.h"

Server::Server(const std::string &address,int port) : address(address), port(port) {
    connection_handler = std::make_shared<ConnectionHandler>(
        this->address, this->port, ConnectionHandlerType::Server
    );
    start();
}

std::string Server::getAddress() {
    if (connection_handler->isInWork())
        return address;
    return "";
}

int Server::getPort() {
    if (connection_handler->isInWork())
        return port;
    return 0;
}

void Server::stop() {
    connection_handler->stop();
}

void Server::start() {
    connection_handler->start();
    connection_handler->setTaskSocket([](std::shared_ptr<tcp::socket> sock) {

        auto& logger = BaseLogger::get();

        TransportHandler transport_handler(sock);

        MessageHandler message_handler;
        ServerRequestResponseHandler request_response_handler(message_handler.creator_message);

        TransportMessage transport_message = transport_handler.read();
        if (transport_message.type != "" && transport_message.type != "error") {
            auto message = message_handler.parse(transport_message);
            if (message != nullptr) {
                auto new_message = request_response_handler.processingRequestResponse(std::move(message));
                TransportMessage new_transport_message = message_handler.serialize(std::move(new_message));
                transport_handler.send(new_transport_message);
            }
        }
    });

    connection_handler->listen();
    std::cout << "Server started. Press Enter to stop...\n";
    std::cin.get();  // ждём нажатия Enter
    connection_handler->stop();
}
