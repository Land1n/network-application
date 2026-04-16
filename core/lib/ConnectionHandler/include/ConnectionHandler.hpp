//
// Created by ivan on 07.03.2026.
//

#pragma once

#include <atomic>
#include <boost/asio.hpp>
#include "ConnectedSocket.hpp"
#include "Logger.hpp"
#include "Logger.hpp"

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

    ConnectionHandler(const std::string &address, int port, ConnectionHandlerType type, bool INFO = true);
    ~ConnectionHandler();

    void start();
    void stop();

    // Методы для сервера
    void listen();
    bool disconnected(ConnectedSocket &connected_socket, bool delete_socket = false);
    ConnectedSocket accept(bool blocking = true);

    // Методы для клиента
    void disconnect(bool needJoinThread = true);
    ConnectedSocket connect(int numTry = 10);

    // Геттеры
    std::vector<ConnectedSocket>& getSockets();
    ConnectedSocket& getSocket();
    std::shared_ptr<tcp::acceptor> getAcceptor();
    bool getIsWork() const;

    // Метод для поиска ConnectedSocket по ID
    ConnectedSocket findConnectedSocket(size_t id);

    // Сеттеры
    void setTaskSocket(std::function<void(ConnectedSocket&)> task);
    void setGenerateID(std::function<size_t()>);

    void setNewConnectionHandler(std::function<void(ConnectedSocket)> h);
    void setCloseConnectionHandler(std::function<void(ConnectedSocket)> h);

private:

    // Методы запуска потоков
    void runAcceptorThread();
    void runConnectionCheckThread();

    // Декоратор над Task`ом
    void runTask(ConnectedSocket& connected_socket);

    // Методы создания нужных объектов
    std::shared_ptr<tcp::acceptor> createAcceptor();
    std::shared_ptr<tcp::socket> createSocket();

    // Метод для проверки подключения ConnectedSocket
    Connection is_connected(ConnectedSocket sock);

    Logger& logger = Logger::getInstance();;
    ConnectionHandlerType type;

    boost::asio::io_context io_context;

    std::string address;
    int port;

    std::atomic<bool> isWork{false};

    std::shared_ptr<tcp::acceptor> acceptor_;

    ConnectedSocket socket_client_;
    std::vector<ConnectedSocket> connected_sockets_;

    std::mutex connection_data_mutex;
    // CallBack`и
    std::function<size_t()> generateID;
    std::function<void(ConnectedSocket&)> task_;
    std::function<void(ConnectedSocket)> newConnection;
    std::function<void(ConnectedSocket)> closeConnection;

    // Умные указатели на потоки
    std::shared_ptr<std::thread> acceptor_thread = nullptr;
    std::shared_ptr<std::thread> connection_thread = nullptr;
};