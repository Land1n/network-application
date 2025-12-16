#include <client_session.hpp>


void ClientSession::startSession()
{
	try{
		
		while(true)
		{
			std::memset(this->data, 0, sizeof(this->data));
			boost::system::error_code error;
			

			size_t bytes_read = socket.read_some(boost::asio::buffer(this->data), error);
			if (error == boost::asio::error::eof)
                	{
                        	std::cout << "Client disconnected: " << this->address << ":" << this->port << std::endl;
                        	break;
                	}
            		else if (error) 
				throw boost::system::system_error(error);
			
			if (bytes_read > 0)
			{
				std::cout << "Client [ " << this->address << ":" << this->port << " ] data = " << data << std::endl;
			}
		
		}
	} catch (std::exception &e){
		std::cerr << "Exception: " << e.what() << std::endl;
	}
}


ClientSession::ClientSession(boost::asio::io_context &io_context, tcp::acceptor &acceptor) : socket(io_context)
{
	try{
		acceptor.accept(this->socket);
		this->address = this->socket.remote_endpoint().address().to_string();
		this->port = this->socket.remote_endpoint().port();
		std::cout << "Client connected: " << this->address << " : " << this->port << std::endl;
	} catch (std::exception &e){
		std::cerr << "Exeption : " << e.what() << std::endl;
		
	}
		
		//	startSession();
}

