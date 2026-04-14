#include "ConnectedSocket.hpp"

ConnectedSocket::ConnectedSocket(std::shared_ptr<tcp::socket> socket, std::size_t id)
    : ptr(socket), address(socket->remote_endpoint().address().to_string()),
      port(socket->remote_endpoint().port()), id(id) {}

std::string ConnectedSocket::getAddressAndPort() {
    return address + ":" + std::to_string(port);
}

ConnectedSocket::ConnectedSocket() : ptr(nullptr), address("0.0.0.0"), port(-1), id(0) {}