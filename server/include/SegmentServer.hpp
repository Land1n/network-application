//
// Created by ivan on 09.03.2026.
//
//
#pragma once

#include "../../clientserveriface/include/clientserveriface/server.h"
// #include "ServerRequestResponseHandler.hpp"
#include "SessionManager/SessionManager.hpp"
#include "IOContextHandler/IOContextHandler.hpp"

class SegmentServer : public Network::Server {
public:
	SegmentServer(const std::string& address, int port, bool multiConnect);
	SegmentServer(int port, bool multiConnect);
	~SegmentServer() override;

	void start() override;
	void stop() override;

	void write(Network::ConnectionId id, const void* data, size_t sz) override;
	void disconnect(Network::ConnectionId id) override;
	void setIdDistributionHandler(IdDistributionHandler h) override;

	void setIOMode(IOMode ioMode);

private:
	bool multiConnect;

	std::string address;
	int port;

	IOMode mode = IOMode::Sync;

	std::atomic<bool> isWork = false;

	IdDistributionHandler IDDistributionHandler;

	std::thread mainThread;

	std::unique_ptr<SessionManager> sessionManager;
	IOContextHandler ioContext;
};