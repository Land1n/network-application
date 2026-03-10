//
// Created by ivan on 07.03.2026.
//

#pragma once

#include <atomic>
#include <boost/asio.hpp>

using tcp = boost::asio::ip::tcp;

class ConnectionHandler {
public:
    ConnectionHandler(std::string &address, int port, std::shared_ptr<boost::asio::io_context> io_context) :
        address(address), port(port), io_context(io_context) {}

    ConnectionHandler(const std::string & string, int port, const std::shared_ptr<boost::asio::io_context>::element_type & element);

    std::shared_ptr<tcp::acceptor> listen(std::shared_ptr<std::atomic<bool>> is_working);
    std::shared_ptr<tcp::socket> accept(std::shared_ptr<tcp::acceptor> acceptor);

    std::shared_ptr<tcp::socket> connect();
private:
    std::string address;
    int port;
    std::shared_ptr<boost::asio::io_context> io_context;
};