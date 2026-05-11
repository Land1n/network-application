#pragma once

#include <atomic>
#include <boost/asio.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

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
inline IdConnectionGenerator defaultGenerator;

class BaseConnectionHandler : public std::enable_shared_from_this<BaseConnectionHandler> {
public:
	BaseConnectionHandler(const std::string& address, int port);
	virtual ~BaseConnectionHandler();

	virtual void start() = 0;
	virtual void stop()  = 0;

	// Общие сеттеры
	void setTaskSocket(std::function<void(ConnectedSocket&)> task);
	void setGenerateID(std::function<size_t()> generator);
	void setNewConnectionHandler(std::function<void(ConnectedSocket)> h);
	void setCloseConnectionHandler(std::function<void(ConnectedSocket)> h);

	// Геттеры
	bool getIsWork() const;

protected:
	// Общие методы
	std::shared_ptr<tcp::socket> createSocket();
	Connection is_connected(const ConnectedSocket& sock);
	void runTask(ConnectedSocket& sock);

	// Чисто виртуальны
	virtual void runConnectionCheckTask() = 0;

	// Члены
	Logger& logger = Logger::getInstance();
	std::string address;
	int port;

	std::atomic<bool> isWork{false};
	boost::asio::io_context io_context;

	// Генератор ID
	std::function<size_t()> generateID;

	// Колбэки
	std::function<void(ConnectedSocket&)> task_;
	std::function<void(ConnectedSocket)> newConnection;
	std::function<void(ConnectedSocket)> closeConnection;

	// Воркеры
	Worker connectionWorker;
	Worker taskWorker;
};