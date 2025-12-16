#include <server.hpp>
#include <iostream>
#include <thread>


Server::Server(std::string address, int port) : address(address), port(port) {}


void Server::start()
{
	try{
		tcp::acceptor acceptor(this->io_context,tcp::endpoint(boost::asio::ip::address::from_string(address), port));
		
		std::cout << "Server start on " << this->address << " : " << this->port << std::endl;
		        std::vector<std::thread> threads;
        
        	for (int i = 0; i < 3; ++i) {
            		client_session.emplace_back(this->io_context, acceptor);
       			threads.emplace_back([this, i]() {
                		client_session[i].startSession();
            		});
        	}
        
        	for (std::thread& t : threads) {
              	        t.join();
        	}
	} catch (std::exception &e) {
		std::cerr << "Exeption: " << e.what() << std::endl;
	}
}




void Server::sendData(ClientSession session&, std::vector<char> message)
	boost::system::error_code error;
	boost::asio::write(session.socket, boost::asio::buffer(message),error);
}
