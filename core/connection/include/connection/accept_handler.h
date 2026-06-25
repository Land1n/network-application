//
// Created by guestuser on 31.05.2026.
//
#pragma once
#include <memory>
#include <boost/asio.hpp>
#include "io_context_handler/io_context_handler.h"

#include "utils/alias.h"
#include "utils/error_handler.h"

class AcceptHandler {
public:
	AcceptHandler(boost::asio::io_context& io, const std::string& address, PortInt port);
	AcceptHandler(boost::asio::io_context& io, PortInt port);
	AcceptHandler(boost::asio::io_context& io, const tcp::endpoint&);
	~AcceptHandler();
	void accept(std::shared_ptr<tcp::socket> socket, IOMode);
	void close();

	void setOnAccept(const CallbackError& callback);
	void setOnClose(const CallbackError& callback);
	void setOnError(const CallbackError& callback);

	void getIOContext();
	void setOptionAcceptor(PortInt port);

protected:
	void sync_accept(std::shared_ptr<tcp::socket>& socket);
	void async_accept(std::shared_ptr<tcp::socket>& socket);

	void setOptionAcceptor(const std::string& address, PortInt port);
	void setOptionAcceptor(const tcp::endpoint& endpoint);

	CallbackError onAccept;
	CallbackError onClose;
	CallbackError onError;

	tcp::acceptor acceptor;
};