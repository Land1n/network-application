#include "connection_handler.hpp"

using boost::asio::ip::tcp;


std::shared_ptr<tcp::acceptor> ConnectionHandler::startListen() {

    boost::system::error_code ec;
    const unsigned short MAX_PORT = 65535;
    
    for (unsigned short _port = port; _port <= MAX_PORT; ++_port) {
        auto acceptor = std::make_shared<tcp::acceptor>(*io_context);
        acceptor->set_option(tcp::acceptor::reuse_address(false), ec);
        acceptor->open(tcp::v4());
        acceptor->bind(tcp::endpoint(boost::asio::ip::address::from_string(address), _port),ec);
        if (!ec) {
            acceptor->listen();
            std::cout << "Server started on " << address << ":" << acceptor->local_endpoint().port()<< std::endl;
            return acceptor;
        }
        else if (ec == boost::asio::error::address_in_use || ec == boost::asio::error::access_denied) { 
            ec.clear();
            continue;
        }
        else {
            *is_working = false;
            std::cerr << "Error: cannot create acceptor: " << ec.message() << std::endl;
            return nullptr;
        }
    }
}

std::shared_ptr<tcp::socket> ConnectionHandler::acceptSocket(std::shared_ptr<tcp::acceptor> acceptor)
{
    std::shared_ptr<tcp::socket> socket = std::make_shared<tcp::socket>(*io_context);
    try {
        acceptor->accept(*socket);
        std::cout << "Client connected: "
                    << socket->remote_endpoint().address().to_string() << ":"
                    << socket->remote_endpoint().port() << std::endl;
        return socket;
    } catch (const std::exception& e) {
        std::cerr << "Accept error: " << e.what() << std::endl;
        return nullptr;
    }
}