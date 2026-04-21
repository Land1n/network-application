#pragma once

#include <atomic>
#include <boost/asio.hpp>
#include "ConnectedSocket.hpp"
#include "Logger.hpp"
#include "Worker.hpp"

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

    // Server
    void listen();
    bool disconnected(ConnectedSocket &connected_socket, bool delete_socket = false);
    ConnectedSocket accept(bool blocking = true);

    // Client
    void disconnect(bool needJoinThread = true);
    ConnectedSocket connect(int numTry = 10);

    // Getters
    std::vector<ConnectedSocket>& getSockets();
    ConnectedSocket& getSocket();
    std::shared_ptr<tcp::acceptor> getAcceptor();
    bool getIsWork() const;
    ConnectedSocket findConnectedSocket(size_t id);

    // Setters
    void setTaskSocket(std::function<void(ConnectedSocket&)> task);
    void setGenerateID(std::function<size_t()> generator);
    void setNewConnectionHandler(std::function<void(ConnectedSocket)> h);
    void setCloseConnectionHandler(std::function<void(ConnectedSocket)> h);

private:
    void runAcceptorTask();       // задача для acceptorWorker – бесконечный цикл accept
    void runConnectionCheckTask(); // задача для connectionWorker – проверка соединений
    void runTask(ConnectedSocket &sock);

    std::shared_ptr<tcp::acceptor> createAcceptor();
    std::shared_ptr<tcp::socket> createSocket();
    Connection is_connected(ConnectedSocket sock);

    Logger& logger = Logger::getInstance();
    ConnectionHandlerType type;
    boost::asio::io_context io_context;
    std::string address;
    int port;

    std::atomic<bool> isWork{false};

    std::shared_ptr<tcp::acceptor> acceptor_;
    ConnectedSocket socket_client_;
    std::vector<ConnectedSocket> connected_sockets_;
    std::mutex connection_data_mutex;

    // Генератор ID – член класса
    IdConnectionGenerator defaultGenerator;
    std::function<size_t()> generateID;

    // Callbacks
    std::function<void(ConnectedSocket&)> task_;
    std::function<void(ConnectedSocket)> newConnection;
    std::function<void(ConnectedSocket)> closeConnection;

    // Workers
    Worker acceptorWorker;    // выполняет accept в цикле
    Worker connectionWorker;  // проверяет соединения
    Worker taskWorker;        // выполняет пользовательские задачи (task_)
};