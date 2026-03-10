//
// Created by ivan on 07.03.2026.
//
#include "ConnectionHandler.hpp"
#include <iostream>

std::shared_ptr<tcp::acceptor> ConnectionHandler::listen(std::shared_ptr<std::atomic<bool>> is_working) {
	boost::system::error_code ec;
	auto acceptor = std::make_shared<tcp::acceptor>(*io_context);
	acceptor->open(tcp::v4());
	acceptor->bind(tcp::endpoint(boost::asio::ip::address::from_string(address), port),ec);
    if (!ec) {
    	acceptor->listen();
        return acceptor;
    }
    else if (ec == boost::asio::error::address_in_use) {
		// TODO: надо сделать пренудительное заняте адресса
        return nullptr;
    }
    else {
    	*is_working = false;
        return nullptr;
    }
}

std::shared_ptr<tcp::socket> ConnectionHandler::accept(std::shared_ptr<tcp::acceptor> acceptor) {
	boost::system::error_code ec;
	auto socket = std::make_shared<tcp::socket>(*io_context);

	acceptor->accept(*socket, ec);
	if(ec){
		std::cout << ec.message() << std::endl;
		return nullptr;
	}
	return socket;
}

std::shared_ptr<tcp::socket> ConnectionHandler::connect() {
	boost::system::error_code ec;
	auto socket = std::make_shared<tcp::socket>(*io_context);
	socket->connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(address), port), ec);
	if(ec)
		return nullptr;
	return socket;
}