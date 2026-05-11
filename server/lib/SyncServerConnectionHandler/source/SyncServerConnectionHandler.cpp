#include "SyncServerConnectionHandler.hpp"
#include <chrono>
#include <thread>

SyncServerConnectionHandler::SyncServerConnectionHandler(const std::string& address, int port) :
    BaseConnectionHandler(address, port)
{}

SyncServerConnectionHandler::~SyncServerConnectionHandler()
{
	if(isWork.load())
		stop();
}

std::shared_ptr<tcp::acceptor> SyncServerConnectionHandler::createAcceptor()
{
	if(!isWork.load()) {
		logger.log(LogLevel::Error, __func__, "Server isWork = false");
		return nullptr;
	}
	auto acceptor = std::make_shared<tcp::acceptor>(io_context);
	acceptor->open(tcp::v4());
	boost::system::error_code ec;
	acceptor->set_option(tcp::acceptor::reuse_address(true));
	acceptor->bind(tcp::endpoint(boost::asio::ip::address::from_string(address), port), ec);
	if(ec && ec != boost::asio::error::address_in_use) {
		logger.log(LogLevel::Error, __func__, "Bind error: " + ec.message());
		stop();
		return nullptr;
	}
	if(ec == boost::asio::error::address_in_use)
		logger.log(LogLevel::Warn, __func__, "Address already in use");
	logger.log(LogLevel::Info, __func__, "Acceptor on " + address + ":" + std::to_string(port));
	return acceptor;
}

void SyncServerConnectionHandler::start()
{
	if(isWork.load())
		return;
	isWork.store(true);
	logger.log(LogLevel::Debug, __func__, "Starting...");
	acceptor_ = createAcceptor();
	if(!acceptor_) {
		logger.log(LogLevel::Error, __func__, "acceptor_ == nullptr");
		stop();
		return;
	}
	acceptorWorker.start();
	connectionWorker.start();
	taskWorker.start();
	logger.log(LogLevel::Info, __func__, "Server started");
}

void SyncServerConnectionHandler::stop()
{
	if(!isWork.load())
		return;
	isWork.store(false);
	logger.log(LogLevel::Debug, __func__, "Stopping...");

	acceptorWorker.stop(true);
	connectionWorker.stop(true);
	taskWorker.stop(true);

	if(acceptor_ && acceptor_->is_open()) {
		boost::system::error_code ec;
		acceptor_->cancel(ec);
		acceptor_->close(ec);
		acceptor_.reset();
	}

	std::vector<ConnectedSocket> to_disconnect;
	{
		std::lock_guard<std::mutex> lock(connection_data_mutex);
		to_disconnect.swap(connected_sockets_);
	}
	for(auto& sock: to_disconnect)
		disconnected(sock, true);

	logger.log(LogLevel::Info, __func__, "Server stopped");
}

void SyncServerConnectionHandler::listen()
{
	if(!isWork.load() || !acceptor_) {
		logger.log(LogLevel::Error, __func__, "Not ready");
		return;
	}
	acceptor_->listen();
	acceptorWorker.addTask([this]() {
		runAcceptorTask();
	});
	connectionWorker.addTask([this]() {
		runConnectionCheckTask();
	});
}

void SyncServerConnectionHandler::runAcceptorTask()
{
	logger.log(LogLevel::Info, __func__,  "Acceptor task started");
	while(isWork.load()) {

		ConnectedSocket cs = accept(false);
		if(cs.ptr) {
			if(task_) {
				taskWorker.addTask([this, cs]() mutable {
					task_(cs);
				});
			}
		}
		else {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
	logger.log(LogLevel::Info, __func__, "Acceptor task finished");
}
void SyncServerConnectionHandler::runConnectionCheckTask()
{
	logger.log(LogLevel::Debug, __func__, "Connection check started");
	auto start = std::chrono::steady_clock::now();
	while(isWork.load()) {
		if(std::chrono::steady_clock::now() - start > std::chrono::seconds(5)) {
			start = std::chrono::steady_clock::now();
			logger.log(LogLevel::Debug, "connection_check",
			           "connected_sockets_.size() = " + std::to_string(connected_sockets_.size()));
		}
		std::vector<ConnectedSocket> socks_copy;
		{
			std::lock_guard<std::mutex> lock(connection_data_mutex);
			socks_copy = connected_sockets_;
		}
		for(auto& sock: socks_copy) {
			if(is_connected(sock) != Connection::Connected) {
				logger.log(LogLevel::Debug, __func__, "Socket " + sock.getAddressAndPort() + " disconnected");
				disconnected(sock, true);
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	logger.log(LogLevel::Debug, __func__, "Connection check finished");
}

ConnectedSocket SyncServerConnectionHandler::accept(bool blocking)
{
	if(!isWork.load() || !acceptor_) {
		logger.log(LogLevel::Error, __func__,"Not ready");
		return ConnectedSocket(nullptr,-1);

	}
	auto socket = createSocket();
	boost::system::error_code ec;
	acceptor_->non_blocking(!blocking);
	acceptor_->accept(*socket, ec);
	if(ec == boost::asio::error::would_block)
		return ConnectedSocket(nullptr,-1);

	if(ec) {
		logger.log(LogLevel::Error, __func__,  ec.message());
		return ConnectedSocket(nullptr,-1);
	}

	ConnectedSocket connected_socket(socket, generateID());
	if(newConnection)
		newConnection(connected_socket);
	logger.log(LogLevel::Info, __func__,
			   "Socket " + connected_socket.getAddressAndPort() +
				   " id: " + std::to_string(connected_socket.id));

	std::lock_guard<std::mutex> lock(connection_data_mutex);
	connected_sockets_.push_back(connected_socket);
	return connected_socket;
}

bool SyncServerConnectionHandler::disconnected(ConnectedSocket& connected_socket, bool delete_socket)
{
	boost::system::error_code ec;
	try {
		if(connected_socket.ptr && connected_socket.ptr->is_open())
			connected_socket.ptr->close(ec);
		if(closeConnection)
			closeConnection(connected_socket);
		if(!ec) {
			logger.log(LogLevel::Info, __func__, "Disconnected " + connected_socket.getAddressAndPort());
		}
		else {
			logger.log(LogLevel::Warn, __func__, "Close error: " + ec.message());
		}
		if(delete_socket) {
			std::lock_guard<std::mutex> lock(connection_data_mutex);
			auto it =
			    std::find_if(connected_sockets_.begin(), connected_sockets_.end(), [&](const ConnectedSocket& cs) {
				    return cs.ptr == connected_socket.ptr;
			    });
			if(it != connected_sockets_.end()) {
				connected_sockets_.erase(it);
				logger.log(LogLevel::Debug, __func__, "Removed from connected_sockets");
			}
		}
		return !ec;
	}
	catch(std::exception& e) {
		logger.log(LogLevel::Error, __func__, e.what());
		return false;
	}
}

std::shared_ptr<tcp::acceptor> SyncServerConnectionHandler::getAcceptor()
{
	return acceptor_;
}
std::vector<ConnectedSocket>& SyncServerConnectionHandler::getSockets()
{
	return connected_sockets_;
}

ConnectedSocket SyncServerConnectionHandler::findConnectedSocket(size_t id) {
	std::lock_guard<std::mutex> lock(connection_data_mutex);
	auto it = std::find_if(connected_sockets_.begin(), connected_sockets_.end(),
						   [id](const ConnectedSocket& cs) { return cs.id == id; });
	if (it != connected_sockets_.end())
		return *it;
	return ConnectedSocket(nullptr, -1);
}