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
#include "sdrlogger/sdrlogger.h"

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


    ConnectionHandler connection_handler(address,port,io_context);

    auto& logger = BaseLogger::get();
    logger.init5Levels();

    logger.setLogLevel("ERROR");
    logger("INFO") << "Starting server on " << address << ":" << port << "\n";
    auto acceptor = connection_handler.listen(is_working);
    if (acceptor == nullptr) {
        stop();
        logger("ERROR") << "Acceptor stopped" << "\n";
    } else
        logger("DEBUG") << "Acceptor started" << "\n";
    while (is_working->load()) {
        auto socket = connection_handler.accept(acceptor);
        if (!socket) {
            logger("WARN") << "Connection socket failed" << "\n";
            continue;
        } else {
            logger("INFO") << "Connection socket accepted " << socket->remote_endpoint().address().to_string() << ":" << socket->remote_endpoint().port() << "\n";
        }

        pool.enqueue([this, socket]() {
            if (this->is_working->load()) {
                auto& logger = BaseLogger::get();

                TransportHandler transport_handler(socket);
                MessageHandler message_handler;
                ServerRequestResponseHandler request_response_handler(message_handler.creator_message);

                TransportMessage transport_message = transport_handler.read();
                if (transport_message.type == "error")
                    logger("ERROR") << "Read request TransportMessage" << "\n";
                else {
                    logger("DEBUG") << "Read request TransportMessage" << "\n";
                    auto message = message_handler.parse(transport_message);
                    if (message == nullptr) {
                        logger("ERROR") << "Message parsing failed" << "\n";
                    } else {
                        logger("DEBUG") << "Message request parsed" << "\n";
                        auto new_message = request_response_handler.processingRequestResponse(std::move(message));
                        logger("DEBUG") << "ProcessingRequestResponse new_message->type:" << new_message->type << "\n";
                        TransportMessage new_transport_message = message_handler.serialize(std::move(new_message));
                        if (new_transport_message.payload.empty() == true)
                            logger("WARN") << "Message response serialized" << "\n";
                        else
                            logger("DEBUG") << "Message response serialized" << "\n";

                        if (transport_handler.send(new_transport_message))
                            logger("DEBUG") << "Send response TransportMessage" << "\n";
                        else
                            logger("ERROR") << "Send response TransportMessage" << "\n";
                    }
                }
            }
        });
    }
}
