#pragma once

#include <iostream>
#include <boost/asio.hpp>

#include <vector>

#include <server_session.hpp>

using boost::asio::ip::tcp;

class Server
{
private:
	std::string address;
	int port;
	boost::asio::io_context io_context;

	std::vector<ServerSession> server_session;

public:
	void start();
	void stop();
	Server(std::string address, int port);	
    	std::string getAddress() const { return address; }
    	int getPort() const { return port; }	
};
