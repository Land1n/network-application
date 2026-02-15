#include "connection_handler.hpp"
#include "thread_pool.hpp"

using boost::asio::ip::tcp;


void ConnectionHandler::start() {
    ThreadPool pool(4);                    

    tcp::acceptor acceptor(io_context);
    try {
        acceptor.open(tcp::v4());
        acceptor.bind(tcp::endpoint(boost::asio::ip::address::from_string(address), port));
        acceptor.listen();
        std::cout << "Server started on " << address << ":" << port << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: cannot create acceptor: " << e.what() << std::endl;
        return;
    }      

    while (is_working.load()) {

        auto socket = std::make_shared<tcp::socket>(io_context);

        try {
            acceptor.accept(*socket);
            std::cout << "Client connected: "
                      << socket->remote_endpoint().address().to_string() << ":"
                      << socket->remote_endpoint().port() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Accept error: " << e.what() << std::endl;
            continue;
        }
        pool.enqueue([this, socket]() {
            while (this->is_working.load()) {
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

void ConnectionHandler::stop() {
    is_working = false;

}