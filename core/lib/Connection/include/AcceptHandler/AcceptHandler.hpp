//
// Created by guestuser on 31.05.2026.
//
#pragma once
#include <memory>
#include <boost/asio.hpp>

#include "ErrorHandler/ErrorHandler.hpp"


enum class TypeAcceptHandler {
	Sync = 0,
	Async = 1,
};

using tcp = boost::asio::ip::tcp;
using AsyncCallBack = std::function<void(error_code)>;


class AcceptHandler : public std::enable_shared_from_this<AcceptHandler>{
public:
	AcceptHandler(boost::asio::io_context& io,const std::string & address, unsigned int port,TypeAcceptHandler type);
	AcceptHandler(boost::asio::io_context& io,const tcp::endpoint& ,TypeAcceptHandler type);
	~AcceptHandler();
	void accept(tcp::socket& socket);

	void setCallback(const AsyncCallBack& callback);
protected:
	void sync_accept(tcp::socket& socket);
	void async_accept(tcp::socket& socket);
	void close();

	void setOptionAcceptor(const std::string& address, int port);
	void setOptionAcceptor(const tcp::endpoint& endpoint);

	AsyncCallBack callback;
	TypeAcceptHandler type;

	tcp::acceptor acceptor;
};