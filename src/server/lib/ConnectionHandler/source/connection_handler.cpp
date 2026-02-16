#include "connection_handler.hpp"

using boost::asio::ip::tcp;


std::shared_ptr<tcp::acceptor> ConnectionHandler::startListen() {
    try {
        auto acceptor = std::make_shared<tcp::acceptor>(*io_context);
        acceptor->open(tcp::v4());
        acceptor->bind(tcp::endpoint(boost::asio::ip::address::from_string(address), port));
        acceptor->listen();
        std::cout << "Server started on " << address << ":" << port << std::endl;
        return acceptor;
    } catch (const std::exception& e) {
        *is_working = false;
        std::cerr << "Error: cannot create acceptor: " << e.what() << std::endl;
        return nullptr;
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