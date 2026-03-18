//
// Created by ivan on 14.03.2026.
//
#pragma once
#include <string>
#include <memory>
#include <boost/asio.hpp>


class Client {
public:
    Client(std::string address, int port);
    void start();
    void stop();
private:
    std::string address;
    int port;
    std::shared_ptr<boost::asio::io_context> io_context = std::make_shared<boost::asio::io_context>();
};
