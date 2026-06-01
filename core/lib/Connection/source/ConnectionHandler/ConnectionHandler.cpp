//
// Created by guestuser on 28.05.2026.
//
#include "ConnectionHandler/ConnectionHandler.hpp"

#include <iostream>

ConnectionHandler::ConnectionHandler(tcp::socket& socket, TypeConnectionHandler type) : socket(socket), type(type)
{}
ConnectionHandler::~ConnectionHandler()
{

	if (socket.is_open()) {
		disconnect();
	}
}

void ConnectionHandler::connect(const tcp::endpoint& endpoint)
{
	if(type == TypeConnectionHandler::Sync)
		sync_connect(endpoint);
	else
		async_connect(endpoint);
}

void ConnectionHandler::connect(const std::string& host, uint16_t port)
{
	auto endpoint = tcp::endpoint(boost::asio::ip::make_address(host), port);
	connect(endpoint);
}

void ConnectionHandler::sync_connect(const tcp::endpoint& endpoint)
{
	socket.connect(endpoint, error);
	if(!ErrorHandler::check_error(error, "ConnectionHandler::connect.sync")) {
		disconnect();
	}
}

void ConnectionHandler::async_connect(const tcp::endpoint& endpoint)
{
	auto self = shared_from_this();
	socket.async_connect(endpoint, [this, self](error_code ec) {
		ErrorHandler::check_error(ec, "ConnectionHandler::connect.async");
		if(ec) {
			disconnect();
			return;
		}
		try {
			if(callback) {
				callback(ec);
				ErrorHandler::check_error(ec, "ConnectionHandler::connect.async{Callback}");
			}
		} catch(const std::exception& e) {
			ErrorHandler::check_error(e, "ConnectionHandler::connect.async");
		}
	});
}

void ConnectionHandler::disconnect()
{
	if (!socket.is_open())
		return;
	socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
	ErrorHandler::check_error(error, "ConnectionHandler::disconnect");
	socket.close();
}
void ConnectionHandler::setCallback(const std::function<void(error_code)>& c)
{
	if(type == TypeConnectionHandler::Sync)
		return;
	callback = c;
}