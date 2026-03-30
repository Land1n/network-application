//
// Created by ivan on 29.03.2026.
//

#pragma once

#include <boost/asio.hpp>

using tcp = boost::asio::ip::tcp;

struct ConnectedSocket {
    std::shared_ptr<tcp::socket> ptr;
    int port;
    std::string address;
    ConnectedSocket(std::shared_ptr<tcp::socket> socket);
    ConnectedSocket();
    std::string getAddressAndPort();
};
