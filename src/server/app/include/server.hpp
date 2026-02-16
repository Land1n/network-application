#pragma once 

#include <iostream>
#include <atomic>
#include <boost/asio.hpp>

#include "thread_pool.hpp"


class Server
{
public:
    Server(std::string address,int port) 
        : address(std::move(address)), port(port)
    {}

    void start();
    // void stop();

private:
    std::shared_ptr<std::atomic<bool>> is_working = std::make_shared<std::atomic<bool>>(true);
    std::shared_ptr<boost::asio::io_context> io_context = std::make_shared<boost::asio::io_context>();
    std::string address;
    int port;
};