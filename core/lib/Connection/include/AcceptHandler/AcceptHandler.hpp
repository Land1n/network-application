//
// Created by guestuser on 31.05.2026.
//
#pragma once
#include <memory>
#include <boost/asio.hpp>
#include "IOContextHandler/IOContextHandler.hpp"

#include "ErrorHandler/ErrorHandler.hpp"
#include "utils.hpp"

class AcceptHandler {
public:
	AcceptHandler(boost::asio::io_context& io, const std::string& address, unsigned int port);
	AcceptHandler(boost::asio::io_context& io, const tcp::endpoint&);
	~AcceptHandler();
	void accept(std::shared_ptr<tcp::socket> socket, IOMode);
	void close();

	void setOnAccept(const CallBack& callback);
	void setOnClose(const CallBack& callback);
	void setOnError(const CallBack& callback);

	void getIOContext();
protected:
	void sync_accept(std::shared_ptr<tcp::socket>& socket);
	void async_accept(std::shared_ptr<tcp::socket>& socket);

	void setOptionAcceptor(const std::string& address, int port);
	void setOptionAcceptor(const tcp::endpoint& endpoint);

	CallBack onAccept;
	CallBack onClose;
	CallBack onError;

	tcp::acceptor acceptor;
};