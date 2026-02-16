#include "server.hpp"
#include "connection_handler.hpp"

void Server::start()
{
    ThreadPool pool(6);
    
    ConnectionHandler connection_handler(this->is_working,address,port,this->io_context);
 
    auto acceptor = connection_handler.startListen(); 

    while (is_working->load())
    {
        auto socket = connection_handler.acceptSocket(acceptor);
        pool.enqueue([this, socket]() {
            while (this->is_working->load()) {
                try {
                    char data[1024];
                    boost::system::error_code error;
                    size_t len = socket->read_some(boost::asio::buffer(data), error);
                    if (error == boost::asio::error::eof)
                	{
                        	std::cout << "Client disconnected: " << socket->remote_endpoint().address().to_string() << ":" << socket->remote_endpoint().port() << std::endl;
                        	break;
                	}
            		else if (error) 
        				throw boost::system::system_error(error);
                    else if (!error) {
                        boost::asio::write(*socket, boost::asio::buffer("ok"));
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Client "
                            << socket->remote_endpoint().address().to_string() << ":"
                            << socket->remote_endpoint().port()
                            << " processing error: " << e.what() << std::endl;
                }
            }
        });
    }
}

