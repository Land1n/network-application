//
// Created by ivan on 09.03.2026.
//

#include "Server.hpp"
#include "ThreadPool.hpp"
#include "ConnectionHandler.hpp"

#include "../include/Server.hpp"

#include <string>


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

    auto acceptor = connection_handler.listen(is_working);
    if (acceptor == nullptr)
        stop();

    while (is_working->load()) {
        auto socket = connection_handler.accept(acceptor);
        if (!socket) {
            continue;
        }

        // auto request_response_handler = std::make_shared<RequestResponseHandler>(is_working, socket);
        // pool.enqueue([request_response_handler]() {
            // request_response_handler->processingData();
        // });
    }
}
