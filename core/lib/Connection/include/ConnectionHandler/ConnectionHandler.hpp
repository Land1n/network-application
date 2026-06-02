//
// Created by guestuser on 28.05.2026.
//

#pragma once

#include <boost/asio.hpp>
#include "ErrorHandler/ErrorHandler.hpp"
#include <functional>
#include "utils.hpp"


class ConnectionHandler : public std::enable_shared_from_this<ConnectionHandler> {
public:
	ConnectionHandler(tcp::socket&);
	~ConnectionHandler();

	void connect(const std::string& host, uint16_t port,IOMode);
	void connect(const tcp::endpoint& endpoint,IOMode);
	void disconnect();

	void setOnConnect(const CallBack&);
	void setOnDisconnect(const CallBack&);
protected:

	void sync_connect(const tcp::endpoint&);

	void async_connect(const tcp::endpoint&);

	tcp::socket& socket;

	CallBack onConnect;
	CallBack onDisconnect;
};
