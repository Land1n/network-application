#pragma once

#include <iostream>
#include <atomic>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class ConnectionHandler {
public:
    explicit ConnectionHandler(std::shared_ptr<std::atomic<bool>> is_working,std::string address, int port, std::shared_ptr<boost::asio::io_context>  io_context)
        : is_working(is_working), address(std::move(address)), port(port),io_context(io_context)
    {}

    std::shared_ptr<tcp::acceptor> startListen();
    std::shared_ptr<tcp::socket> acceptSocket(std::shared_ptr<tcp::acceptor> acceptor);
private:
    std::shared_ptr<std::atomic<bool>> is_working;
    std::shared_ptr<boost::asio::io_context> io_context;
    std::string address;
    int port;
};