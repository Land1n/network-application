//
// Created by ivan on 29.03.2026.
//

#pragma once

#include "../../TransportHandler/include/TransportHandler/TransportHandler.hpp"

#include <boost/asio.hpp>
#include <memory>
#include <string>

using tcp = boost::asio::ip::tcp;

struct ConnectedSocket {
    std::size_t id;
    std::shared_ptr<tcp::socket> ptr;
    int port;
    std::string address;
    ConnectedSocket(std::shared_ptr<tcp::socket> socket, std::size_t id);
    const std::string getAddressAndPort();
};