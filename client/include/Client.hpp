//
// Created by ivan on 14.03.2026.
//
#pragma once
#include <string>
#include <memory>
#include <boost/asio.hpp>

#include "ConnectionHandler.hpp"

class Client {
public:
    Client(std::string address, int port);
    void start();
    void stop();

    ~Client();

private:
    std::string address;
    int port;
    std::shared_ptr<ConnectionHandler> connection_handler;
};
