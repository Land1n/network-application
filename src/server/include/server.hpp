#pragma once

#include <iostream>
#include <boost/asio.hpp>

#include <vector>

#include <client_session.hpp>

using boost::asio::ip::tcp;

class Server
{
private:
	std::string address;
	int port;
	boost::asio::io_context io_context;

	std::vector<ClientSession> client_session;

public:
	void start();
	void stop();
	Server(std::string address, int port);	
	//void sendData(ClientSession,std::vector<char>);
};
