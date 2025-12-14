#include <boost/asio.hpp>
#include <iostream>
#include <thread>

using boost::asio::ip::tcp;


void handle_client(tcp::socket socket) {
    std::cout << "Client connected: "
    << socket.remote_endpoint().address().to_string() << ":"
    << socket.remote_endpoint().port() << std::endl;
    try {
        char data[1024];
        while (true) {
            std::memset(data, 0, sizeof(data));
            boost::system::error_code error;

            // Read message from client
            size_t length = socket.read_some(boost::asio::buffer(data), error);
	    if (error == boost::asio::error::eof)
		{
			std::cout << "Client disconnected: " 
                	<< socket.remote_endpoint().address().to_string() << ":" 
                	<< socket.remote_endpoint().port() << std::endl;   
			break;
		}
            else if (error) throw boost::system::system_error(error);

            std::cout << "Client " << socket.remote_endpoint().address().to_string() << ":"
                        << socket.remote_endpoint().port()<<" = " << data << std::endl;

            // Send response
            std::string response;
            std::cout << "Answer for " << socket.remote_endpoint().address().to_string() << ":"
                        << socket.remote_endpoint().port() <<" = " ;
            std::getline(std::cin, response);
            boost::asio::write(socket, boost::asio::buffer(response), error);
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}


void server(short n_client = 3)
{
    try {
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 12345));

        std::cout << "Server started. Waiting for client..." << std::endl;


        std::vector<std::thread> threads;

        for (short i = 0; i < n_client; ++i)
        {
                tcp::socket socket(io_context);
                acceptor.accept(socket);
                threads.emplace_back(handle_client, std::move(socket));
        }


        for (std::thread& t : threads) {
                if (t.joinable()) {
                   t.join(); // Блокирует до завершения потока
                }
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}


int main() 
{
	server();
	return 0;
}
