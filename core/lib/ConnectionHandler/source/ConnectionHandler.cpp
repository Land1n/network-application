//
// Created by ivan on 07.03.2026.
//
#include "ConnectionHandler.hpp"
#include <iostream>

#include "sdrlogger/sdrlogger.h"

ConnectionHandler::ConnectionHandler(std::string &address, int port, std::shared_ptr<boost::asio::io_context> io_context,bool DEBUG) :
	address(address), port(port), io_context(io_context)
{
	logger.init5Levels();
	if (!DEBUG)
		logger.setLogLevel("ERROR");
	else
		logger.setLogLevel("DEBUG");
}


std::shared_ptr<tcp::acceptor> ConnectionHandler::listen(std::shared_ptr<std::atomic<bool>> is_working) {
	boost::system::error_code ec;
	auto acceptor = std::make_shared<tcp::acceptor>(*io_context);
	acceptor->open(tcp::v4());
	acceptor->set_option(tcp::acceptor::reuse_address(true));
	acceptor->bind(tcp::endpoint(boost::asio::ip::address::from_string(address), port),ec);
    if (!ec || ec == boost::asio::error::address_in_use) {
		if (ec == boost::asio::error::address_in_use)
			logger("WARN") << "ConnectionHandler : Address in use" << "\n";
    	acceptor->listen();
    	logger("DEBUG") << "ConnectionHandler : Acceptor started" << "\n";
    	return acceptor;
    } else {
        logger("ERROR") << "ConnectionHandler : Acceptor stopped" << "\n";
    	*is_working = false;
        return nullptr;
    }
}

std::shared_ptr<tcp::socket> ConnectionHandler::accept(std::shared_ptr<tcp::acceptor> acceptor) {
	boost::system::error_code ec;
	auto& logger = BaseLogger::get();
	auto socket = std::make_shared<tcp::socket>(*io_context);

	acceptor->accept(*socket, ec);
	if(ec){
		logger("WARN") << "ConnectionHandler : Accept socket "<< socket->remote_endpoint().address().to_string() << ":" << socket->remote_endpoint().port()  << "\n";
		std::cout << ec.message() << std::endl;
		return nullptr;
	}
	logger("INFO") << "ConnectionHandler : Accept socket "<< socket->remote_endpoint().address().to_string() << ":" << socket->remote_endpoint().port()<< "\n";
	return socket;
}

std::shared_ptr<tcp::socket> ConnectionHandler::connect() {
	boost::system::error_code ec;
	auto& logger = BaseLogger::get();
	auto socket = std::make_shared<tcp::socket>(*io_context);
	socket->connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(address), port), ec);
	if(ec) {
		logger("WARN") << "ConnectionHandler : Connection socket "<< socket->remote_endpoint().address().to_string() << ":" << socket->remote_endpoint().port() << "\n";
		return nullptr;
	}
	logger("INFO") << "ConnectionHandler : Connection socket " << socket->remote_endpoint().address().to_string() << ":" << socket->remote_endpoint().port() << "\n";
	return socket;
}

bool ConnectionHandler::disconnected(std::shared_ptr<tcp::socket> socket) {
	boost::system::error_code ec;
	std::string adr = socket->remote_endpoint().address().to_string();
	int port = socket->remote_endpoint().port();
	socket->close(ec);
	if (!ec) {
		logger("INFO") << "ConnectionHandler : Disconnected socket "<< adr << ":" << port<< "\n";
		return true;
	} else {
		logger("WARN") << "ConnectionHandler : Disconnected socket "<< adr << ":" << port<< "\n";
		return false;
	}
}