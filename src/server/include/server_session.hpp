#pragma once

#include <iostream>
#include <boost/asio.hpp>

#include <../core/include/request_response_handler.hpp>


using boost::asio::ip::tcp;

class ServerSession
{
public:
	
	ServerSession(boost::asio::io_context &io_context,tcp::acceptor &acceptor);	
	void startSession();
	void processingCommand(const char* data, std::function<void(const std::string&,const std::string&)> callback);
	void sendData(const std::string& command, const std::string& data);
private:
	int port;
	std::string address;
	char data[1024];
	tcp::socket socket;

	RequestResponseHandler rrh;
 
};

