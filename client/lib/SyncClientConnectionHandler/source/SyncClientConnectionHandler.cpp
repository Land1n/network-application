//
// Created by ivan on 06.05.2026.
//
#include "SyncClientConnectionHandler.hpp"
#include <chrono>
#include <thread>

SyncClientConnectionHandler::SyncClientConnectionHandler(const std::string& address, int port) :
    BaseConnectionHandler(address, port)
{}

SyncClientConnectionHandler::~SyncClientConnectionHandler()
{
	stop();
}


void SyncClientConnectionHandler::start()
{
	if(isWork.load())
		return;
	logger.log(LogLevel::Debug, __func__, "Client starting...");
	isWork.store(true);
	logger.log(LogLevel::Info, __func__, "Client started");
}

void SyncClientConnectionHandler::stop()
{
	if(!isWork.load())
		return;
	isWork.store(false);
	logger.log(LogLevel::Debug, __func__, "Stopping...");
	connectionWorker.flush();
	taskWorker.flush();
	if(socket_client.ptr && socket_client.ptr->is_open()) {
		boost::system::error_code ec;
		socket_client.ptr->close(ec);
		if(closeConnection)
			closeConnection(socket_client);
		socket_client.ptr.reset();
	}
	logger.log(LogLevel::Info, __func__, "Client stopped");
}

ConnectedSocket SyncClientConnectionHandler::connect(int numTry)
{
	if(!isWork.load()) {
		logger.log(LogLevel::Error, __func__, "Client not started");
		return ConnectedSocket(nullptr, -1);
	}
	boost::system::error_code ec;
	for(int i = 1; i <= numTry; ++i) {
		logger.log(LogLevel::Debug, __func__, "Try " + std::to_string(i));
		socket_client.ptr = createSocket();
		socket_client.ptr->connect(tcp::endpoint(boost::asio::ip::address::from_string(address), port), ec);
		if(!ec) {
			logger.log(LogLevel::Info, __func__,
			           "Connected to " + socket_client.ptr->remote_endpoint().address().to_string() + ":" +
			               std::to_string(socket_client.ptr->remote_endpoint().port()));
			socket_client.id = generateID();
			connectionWorker.start();
			connectionWorker.addTask([this]() {
				runConnectionCheckTask();
			});
			if(task_) {
				taskWorker.addTask([this, sock = socket_client]() mutable {
					runTask(sock);
				});
			}
			if(newConnection)
				newConnection(socket_client);
			return socket_client;
		}
		logger.log(LogLevel::Warn, __func__, "Connect failed: " + ec.message());
		if(i == numTry)
			stop();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	return ConnectedSocket(nullptr, -1);
}

void SyncClientConnectionHandler::disconnect(bool needJoinThread)
{
	if(!socket_client.ptr || !socket_client.ptr->is_open()) {
		logger.log(LogLevel::Debug, __func__, "Socket already closed");
		return;
	}
	isWork.store(false);
	if(needJoinThread)
		connectionWorker.stop(true);
	boost::system::error_code ec;
	socket_client.ptr->close(ec);
	if(closeConnection)
		closeConnection(socket_client);
	if(ec)
		logger.log(LogLevel::Error, __func__, "Close error: " + ec.message());
	else
		logger.log(LogLevel::Info, __func__, "Disconnected from " + address + ":" + std::to_string(port));
}

void SyncClientConnectionHandler::runConnectionCheckTask()
{
	logger.log(LogLevel::Debug, __func__, "Connection check started");
	while(isWork.load()) {
		if(socket_client.ptr && is_connected(socket_client) != Connection::Connected) {
			logger.log(LogLevel::Info, __func__, "Connection lost");
			disconnect(false);
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	logger.log(LogLevel::Debug, __func__, "Connection check finished");
}

ConnectedSocket& SyncClientConnectionHandler::getSocket()
{
	return socket_client;
}