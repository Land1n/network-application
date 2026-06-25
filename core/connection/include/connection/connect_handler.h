//
// Created by guestuser on 28.05.2026.
//

#pragma once

#include <boost/asio.hpp>
#include "utils/error_handler.h"
#include "utils/alias.h"
#include <functional>
#include "io_context_handler/io_context_handler.h"

class ConnectHandler {
public:
	ConnectHandler(std::shared_ptr<tcp::socket>&);
	~ConnectHandler();

	void connect(const std::string& host, PortInt port, IOMode);
	void connect(const tcp::endpoint& endpoint, IOMode);
	void disconnect();

	void setOnConnect(const CallbackError&);
	void setOnDisconnect(const CallbackError&);
	void setOnError(const CallbackError&);

protected:
	void sync_connect(const tcp::endpoint&);

	void async_connect(const tcp::endpoint&);

	std::shared_ptr<tcp::socket> socket;

	CallbackError onConnect;
	CallbackError onDisconnect;

	CallbackError onError;
};
