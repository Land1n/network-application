#pragma once

#include <iostream>
#include <boost/asio.hpp>


using boost::asio::ip::tcp;

class ClientSession
{
public:
	
	ClientSession(boost::asio::io_context &,tcp::acceptor &);	
	void startSession();
private:
	int port;
	std::string address;
	char data[1024];
	tcp::socket socket;
};

