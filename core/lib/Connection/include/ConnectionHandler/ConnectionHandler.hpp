//
// Created by guestuser on 28.05.2026.
//

#pragma once

#include <boost/asio.hpp>
#include "ErrorHandler/ErrorHandler.hpp"
#include <functional>

using tcp = boost::asio::ip::tcp;
using AsyncCallBack = std::function<void(error_code)>;

enum class TypeConnectionHandler {
	Sync = 0,
	Async = 1,
};

class ConnectionHandler : public std::enable_shared_from_this<ConnectionHandler> {
public:
	ConnectionHandler(tcp::socket&, TypeConnectionHandler);
	~ConnectionHandler();

	void connect(const std::string& host, uint16_t port);
	void connect(const tcp::endpoint& endpoint);
	void disconnect();

	void setCallback(const AsyncCallBack&);

protected:

	void sync_connect(const tcp::endpoint&);

	void async_connect(const tcp::endpoint&);

	tcp::socket& socket;
	boost::system::error_code error;
	TypeConnectionHandler type;

	AsyncCallBack callback;
};
