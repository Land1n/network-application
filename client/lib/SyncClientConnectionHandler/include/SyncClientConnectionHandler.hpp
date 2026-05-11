//
// Created by ivan on 06.05.2026.
//
#pragma once

#include "BaseConnectionHandler.hpp"

class SyncClientConnectionHandler : public BaseConnectionHandler {
public:
	SyncClientConnectionHandler(const std::string& address, int port);
	~SyncClientConnectionHandler() override;

	void start() override;
	void stop() override;

	ConnectedSocket connect(int numTry = 10);
	void disconnect(bool needJoinThread = true);

	ConnectedSocket& getSocket();

protected:
	void runConnectionCheckTask() override;

	ConnectedSocket socket_client{nullptr,0};
};
