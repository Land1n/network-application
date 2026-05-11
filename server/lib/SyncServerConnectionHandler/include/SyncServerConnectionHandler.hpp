#pragma once

#include "BaseConnectionHandler.hpp"
#include <memory>

class SyncServerConnectionHandler : public BaseConnectionHandler {
public:
	SyncServerConnectionHandler(const std::string& address, int port);
	~SyncServerConnectionHandler() override;

	void start() override;
	void stop() override;

	void listen();
	bool disconnected(ConnectedSocket& connected_socket, bool delete_socket = false);
	ConnectedSocket accept(bool blocking = true);

	std::shared_ptr<tcp::acceptor> getAcceptor();
	std::vector<ConnectedSocket>& getSockets();
	ConnectedSocket findConnectedSocket(size_t id);

protected:
	void runAcceptorTask();
	void runConnectionCheckTask() override;

	std::shared_ptr<tcp::acceptor> createAcceptor();

	std::shared_ptr<tcp::acceptor> acceptor_;
	std::vector<ConnectedSocket> connected_sockets_;

	std::mutex connection_data_mutex;

	Worker acceptorWorker;
};