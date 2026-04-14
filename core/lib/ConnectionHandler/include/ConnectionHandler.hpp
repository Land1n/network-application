//
// Created by ivan on 07.03.2026.
//

#pragma once

#include <atomic>
#include <boost/asio.hpp>
#include "ConnectedSocket.hpp"
#include "Logger.hpp"
#include "LoggerFactory.hpp"

using tcp = boost::asio::ip::tcp;

enum class ConnectionHandlerType { Client = 0, Server = 1 };
enum class Connection { Connected = 0, Disconnected = 1, Error = -1 };

class IdConnectionGenerator {
    std::atomic<size_t> next{0};
public:
    size_t generate() { return next.fetch_add(1); }
};

class ConnectionHandler : public std::enable_shared_from_this<ConnectionHandler> {
public:
    ConnectionHandler(std::string &address, int port, ConnectionHandlerType type, bool INFO = true);
    ~ConnectionHandler();

    void start();
    void stop();

    void setTaskSocket(std::function<void(std::shared_ptr<tcp::socket>)> task);

    // Методы для сервера
    void listen();
    bool disconnected(ConnectedSocket &connected_socket, bool delete_socket = false);
    std::shared_ptr<tcp::socket> accept(bool blocking = true);
    // Методы для клиента
    void disconnect(bool needJoinThread = true);
    std::shared_ptr<tcp::socket> connect(int numTry = 10);

    // Геттеры
    std::vector<ConnectedSocket>& getSockets();
    ConnectedSocket getSocket();
    std::shared_ptr<tcp::acceptor> getAcceptor();

    bool isInWork() const;

    void runAcceptorThread();

    void runConnectionCheckThread();

private:

    void runTask(std::shared_ptr<tcp::socket> socket);

    std::shared_ptr<tcp::acceptor> createAcceptor();
    std::shared_ptr<tcp::socket> createSocket();

    Connection is_connected(ConnectedSocket sock);

    std::shared_ptr<Logger> logger;
    ConnectionHandlerType type;
    boost::asio::io_context io_context;
    std::string address;
    int port;
    std::atomic<bool> isWork{false};
    std::function<void(std::shared_ptr<tcp::socket>)> task_;

    std::shared_ptr<tcp::acceptor> acceptor_;
    ConnectedSocket socket_client_;
    std::vector<ConnectedSocket> connected_sockets_;

    std::mutex connection_data_mutex;

    IdConnectionGenerator generator;

    std::shared_ptr<std::thread> acceptor_thread = nullptr;
    std::shared_ptr<std::thread> connection_thread = nullptr;
};