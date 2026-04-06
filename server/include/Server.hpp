//
// Created by ivan on 09.03.2026.
//

#pragma once
#include <string>

#include "ConnectionHandler.hpp"

class Server {
public:
    Server(const std::string &address,int port);
    void start();
    void stop();

    std::string getAddress();
    int getPort();
private:
    std::string address;
    int port;
    std::shared_ptr<ConnectionHandler> connection_handler;
};
