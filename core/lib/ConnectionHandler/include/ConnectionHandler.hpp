//
// Created by ivan on 07.03.2026.
//

#pragma once

#include <atomic>
#include <boost/asio.hpp>
#include "ConnectedSocket.hpp"
#include "sdrlogger/sdrlogger.h"

using tcp = boost::asio::ip::tcp;

enum class ConnectionHandlerType {
    Client,
    Server,
};


class ConnectionHandler : public std::enable_shared_from_this<ConnectionHandler>  {
public:
    ConnectionHandler(std::string &address,int port, ConnectionHandlerType type, bool INFO = true);

    ~ConnectionHandler();

    void start(); // Запускает работу
    void stop(); // Останавливает все операции

    bool isInWork() const;

    void listen();
    bool disconnected(ConnectedSocket &connected_socket, bool delete_socket = false);

    // createAcceptor - метод, который создает ацептор для подлючения
    std::shared_ptr<tcp::acceptor> createAcceptor();
    std::shared_ptr<tcp::socket> createSocket();

    std::shared_ptr<tcp::socket> accept(bool blocking = true);

    void setTaskSocket(std::function<void(std::shared_ptr<tcp::socket>)> task);

    bool is_connected(std::shared_ptr<tcp::socket> sock);

    // Методы для клиента
    void disconnect();
    std::shared_ptr<tcp::socket> connect();

    // Геттеры
    std::vector<ConnectedSocket>& getSockets();
    std::shared_ptr<tcp::socket> getSocket();
    std::shared_ptr<tcp::acceptor> getAcceptor();


private:
    BaseLogger &logger = BaseLogger::get();

    ConnectionHandlerType type;

    boost::asio::io_context io_context;

    std::string address;
    int port;

    std::atomic<bool> isWork {false};

    std::function<void(std::shared_ptr<tcp::socket>)> task_;

    std::shared_ptr<tcp::acceptor> acceptor_;
    std::shared_ptr<tcp::socket> socket_client_;
    std::vector<ConnectedSocket> connected_sockets_ = {};

    std::shared_ptr<std::thread> acceptor_thread = nullptr;
    std::shared_ptr<std::thread> connection_thread = nullptr;
};
