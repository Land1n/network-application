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
    auto thread_pool = std::make_shared<ThreadPool>(8);
    auto connection_handler = std::make_shared<ConnectionHandler>(
        address, port, ConnectionHandlerType::Server,thread_pool
    );
    connection_handler->listen();
    std::cout << "Server started. Press Enter to stop...\n";
    std::cin.get();  // ждём нажатия Enter
    connection_handler->stop();
    // std::thread cheak_thread([ &connection_handler]() {
    //     connection_handler.checkConnection();
    // });
    // std::thread stop_thread([ &connection_handler]() {
    //     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    //     connection_handler.stop();
    // });

    // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // stop_thread.join();

    //
    //  is_working->store(true);
    //
    //
    // ConnectionHandler connection_handler(address,port,io_context);
    //
    // auto& logger = BaseLogger::get();
    // logger.init5Levels();
    // logger.setLogLevel("ERROR");
    //
    // logger("INFO") << "Starting server on " << address << ":" << port << "\n";
    // auto acceptor = connection_handler.listen();
    // if (acceptor == nullptr) {
    //     stop();
    // }
    // while (is_working->load()) {
    //     auto socket = connection_handler.accept(acceptor);
    //     if (!socket)
    //         continue;
    //
    //     pool.enqueue([this, socket, &connection_handler]() {
    //         if (this->is_working->load()) {
    //             auto& logger = BaseLogger::get();
    //
    //             TransportHandler transport_handler(socket);
    //             MessageHandler message_handler;
    //             ServerRequestResponseHandler request_response_handler(message_handler.creator_message);
    //
    //             TransportMessage transport_message = transport_handler.read();
    //             if (transport_message.type != "error"){
    //                 auto message = message_handler.parse(transport_message);
    //                 if (message != nullptr) {
    //                     auto new_message = request_response_handler.processingRequestResponse(std::move(message));
    //                     TransportMessage new_transport_message = message_handler.serialize(std::move(new_message));
    //                     transport_handler.send(new_transport_message);
    //                 }
    //             }
    //             connection_handler.disconnected(socket);
    //         }
    //     });
    // }
}
