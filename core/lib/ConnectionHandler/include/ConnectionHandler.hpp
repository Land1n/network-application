//
// Created by ivan on 07.03.2026.
//

#pragma once

#include <atomic>
#include <boost/asio.hpp>

#include "sdrlogger/sdrlogger.h"

using tcp = boost::asio::ip::tcp;

class ConnectionHandler {
public:
    ConnectionHandler(std::string &address, int port, std::shared_ptr<boost::asio::io_context> io_context,bool DEBUG = false);

    // Server функции
    std::shared_ptr<tcp::acceptor> listen(std::shared_ptr<std::atomic<bool>> is_working);
    std::shared_ptr<tcp::socket> accept(std::shared_ptr<tcp::acceptor> acceptor);

    // Client функции
    std::shared_ptr<tcp::socket> connect();
    bool disconnected(std::shared_ptr<tcp::socket> socket);
private:
	BaseLogger& logger = BaseLogger::get();
    std::string address;
    int port;
    std::shared_ptr<boost::asio::io_context> io_context;
};