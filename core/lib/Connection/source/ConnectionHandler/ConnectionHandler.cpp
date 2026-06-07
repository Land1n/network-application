//
// Created by guestuser on 28.05.2026.
//
#include "ConnectionHandler/ConnectionHandler.hpp"

#include <iostream>

ConnectionHandler::ConnectionHandler(std::shared_ptr<tcp::socket>& socket) : socket(socket)
{}
ConnectionHandler::~ConnectionHandler()
{
	if(!socket || !socket->is_open()) {
		boost::system::error_code ec;
		ErrorHandler::check_error(ec, "ConnectionHandler::~ConnectionHandler");
		return;
	}
	auto s = socket;
	boost::asio::post(s->get_executor(), [s]() {
		boost::system::error_code ec;
		s->shutdown(tcp::socket::shutdown_both, ec);
		ErrorHandler::check_error(ec, "ConnectionHandler::~ConnectionHandler{shutdown}");
		s->close(ec);
		ErrorHandler::check_error(ec, "ConnectionHandler::~ConnectionHandler{close}");
	});
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
	socket->connect(endpoint, error);
	if(ErrorHandler::check_error(error, "ConnectionHandler::connect.sync")) {
		if(onError) {
			onError(error);
		}
		disconnect();
	}
	if(onConnect) {
		onConnect(error);
	}
}

void ConnectionHandler::async_connect(const tcp::endpoint& endpoint)
{
	socket->async_connect(endpoint, [this](error_code ec) {
		try {
			if(ErrorHandler::check_error(ec, "ConnectionHandler::connect.async")) {
				if(onError) {
					onError(ec);
				}
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
	if(!socket->is_open())
		return;

	error_code ec;
	socket->shutdown(tcp::socket::shutdown_both, ec);
	if(ec) {
		if(onError)
			onError(ec);
		ErrorHandler::check_error(ec, "ConnectionHandler::disconnect(shutdown)");
	}

	socket->close(ec);
	if(ec) {
		if(onError)
			onError(ec);
		ErrorHandler::check_error(ec, "ConnectionHandler::disconnect(close)");
	}

	if(onDisconnect)
		onDisconnect(ec);
}
void ConnectionHandler::setOnConnect(const CallBack& c)
{
	onConnect = c;
}
void ConnectionHandler::setOnDisconnect(const CallBack& c)
{
	onDisconnect = c;
}
void ConnectionHandler::setOnError(const CallBack& c)
{
	onError = c;
}