//
// Created by guestuser on 28.05.2026.
//
#include "ConnectionHandler/ConnectionHandler.hpp"

#include <iostream>

ConnectionHandler::ConnectionHandler(tcp::socket& socket) : socket(socket)
{}
ConnectionHandler::~ConnectionHandler()
{
	disconnect();
}

void ConnectionHandler::connect(const tcp::endpoint& endpoint, IOMode mode)
{
	if(mode == IOMode::Sync)
		sync_connect(endpoint);
	else {
		async_connect(endpoint);
	}
}

void ConnectionHandler::connect(const std::string& host, uint16_t port, IOMode mode)
{
	auto endpoint = tcp::endpoint(boost::asio::ip::make_address(host), port);
	connect(endpoint, mode);
}

void ConnectionHandler::sync_connect(const tcp::endpoint& endpoint)
{
	error_code error;
	socket.connect(endpoint, error);
	if(ErrorHandler::check_error(error, "ConnectionHandler::connect.ssync")) {
		disconnect();
	}
	if(onConnect) {
		onConnect(error);
	}
}

void ConnectionHandler::async_connect(const tcp::endpoint& endpoint)
{
	auto self = shared_from_this();
	socket.async_connect(endpoint, [this, self](error_code ec) {
		try {
			if(ErrorHandler::check_error(ec, "ConnectionHandler::connect.async")) {
				disconnect();
			}
			if(onConnect) {
				onConnect(ec);
			}
		}
		catch(const std::exception& e) {
			ErrorHandler::check_error(e, "ConnectionHandler::connect.async");
		}
	});
}

void ConnectionHandler::disconnect()
{
	error_code error;
	if(socket.is_open()) {
		socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
		socket.close();
		if(onDisconnect) {
			onDisconnect(error);
		}
		ErrorHandler::check_error(error, "ConnectionHandler::disconnect");
	}
}
void ConnectionHandler::setOnConnect(const CallBack& c)
{
	onConnect = c;
}
void ConnectionHandler::setOnDisconnect(const CallBack& c)
{
	onDisconnect = c;
}