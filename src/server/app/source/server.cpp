#include "server.hpp"
#include "connection_handler.hpp"
#include "request_response_handler.hpp"

void Server::start()
{
    ThreadPool pool(6);
    
    ConnectionHandler connection_handler(this->is_working,address,port,this->io_context);
 
    auto acceptor = connection_handler.startListen(); 

    if (!acceptor) {
        std::cerr << "Failed to start listening" << std::endl;
        return;
    }

    while (is_working->load())
    {
        auto socket = connection_handler.acceptSocket(acceptor);
        if (!socket) {
            // ошибка, можно залогировать и продолжить
            continue;
        }
      
        auto request_response_handler = std::make_shared<RequestResponseHandler>(is_working, socket);
        pool.enqueue([request_response_handler]() {
            request_response_handler->processingData();
        });
    }
}

