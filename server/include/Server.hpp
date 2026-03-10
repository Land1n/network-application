//
// Created by ivan on 09.03.2026.
//

#pragma once
#include <string>
#include <boost/asio.hpp>
#include <atomic>

class Server {
public:
    Server(const std::string &address,int port) : address(address), port(port) {};
    void start();
    void stop();

    std::string getAddress();
    int getPort();
private:
    std::string address;
    int port;

    std::shared_ptr<boost::asio::io_context> io_context = std::make_shared<boost::asio::io_context>();
    std::shared_ptr<std::atomic<bool>> is_working = std::make_shared<std::atomic<bool>>(false);
};
