//
// Created by ivan on 09.03.2026.
//

#include <string>

#include "Server.hpp"

#include <iostream>

#include "ThreadPool.hpp"
#include "ConnectionHandler.hpp"
#include "TransportHandler.hpp"
#include "MessageHandler.hpp"
#include "ServerRequestResponseHandler.hpp"


std::string Server::getAddress() {
    if (is_working->load())
        return address;
    return "";
}

int Server::getPort() {
    if (is_working->load())
        return port;
    return 0;
}

void Server::stop() {
    if (is_working->load())
        is_working->store(false);
}

void Server::start() {
    ThreadPool pool(6);

     is_working->store(true);

    std::cout << "Server started" << std::endl;
    ConnectionHandler connection_handler(address,port,io_context);

    auto acceptor = connection_handler.listen(is_working);
    if (acceptor == nullptr)
        stop();
    std::cout << "Acceptor started" << std::endl;
    while (is_working->load()) {
        auto socket = connection_handler.accept(acceptor);
        if (!socket) {
            continue;
        }

        pool.enqueue([this, socket]() {
            while (this->is_working->load()) {
                std::cout << "Connection accepted" << std::endl;
                TransportHandler transport_handler(socket);
                MessageHandler message_handler;
                ServerRequestResponseHandler request_response_handler(message_handler.creator_message);

                TransportMessage transport_message = transport_handler.read();
                std::cout << "Message received" << std::endl;
                auto message = message_handler.parse(transport_message);
                std::cout << "message->type: " << message->type << std::endl;
                std::cout << "message->transactionType: " << static_cast<int>(message->transactionType) << std::endl;
                auto new_message = request_response_handler.processingRequestResponse(std::move(message));
                TransportMessage new_transport_message = message_handler.serialize(std::move(new_message));
                if (transport_handler.send(new_transport_message))
                    std::cout << "Message sent successfully" << std::endl;
                else
                    std::cout << "Message not sent" << std::endl;

            }
        });
    }
}
