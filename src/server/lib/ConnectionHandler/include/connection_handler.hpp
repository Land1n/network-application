#pragma once

#include <iostream>
#include <atomic>
#include <boost/asio.hpp>


class ConnectionHandler {
public:
    ConnectionHandler(std::string address, int port)
        : is_working(true), address(std::move(address)), port(port)
    {}

    void start();
    void stop();

private:
    std::atomic<bool> is_working;
    boost::asio::io_context io_context;
    std::string address;
    int port;
};