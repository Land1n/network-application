#include "ConnectedSocket.hpp"

ConnectedSocket::ConnectedSocket(std::shared_ptr<tcp::socket> socket, std::size_t id) : ptr(socket), id(id)
{
	if(socket != nullptr) {
		auto end = socket->remote_endpoint();
		address  = end.address().to_string();
		port     = end.port();
	}
	else {
		address = "0.0.0.0";
		port    = -1;
	}
}

std::string ConnectedSocket::getAddressAndPort()
{
	return address + ":" + std::to_string(port);
}