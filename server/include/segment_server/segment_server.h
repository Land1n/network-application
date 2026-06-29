//
// Created by ivan on 09.03.2026.
//
#pragma once

#include "clientserveriface/server.h"
#include "session/session_manager.h"
#include "io_context_handler/io_context_handler.h"
#include "worker/worker.h"

class SegmentServer : public Network::Server {
public:
	SegmentServer(const std::string& address, PortInt port, bool multiConnect);
	SegmentServer(int port, bool multiConnect);
	~SegmentServer() override;

	void start() override;
	void stop() override;

	void write(Network::ConnectionId id, const void* data, size_t sz) override;
	void disconnect(Network::ConnectionId id) override;
	void setIdDistributionHandler(IdDistributionHandler h) override;

	void setIOMode(IOMode ioMode);

	void setMagicNumber(uint32_t magicNumber);

	bool getIsWork();

	std::vector<size_t> getSessionVectorID();

private:
	void sessionWork(size_t sessionId);

	bool multiConnect;

	std::string address;
	PortInt port;

	IOMode mode = IOMode::Sync;

	std::atomic<bool> isWork = false;

	IdDistributionHandler IDDistributionHandler;

	Worker mainWorker;

	std::vector<std::unique_ptr<Worker>> sessionWorkers;

	std::unique_ptr<SessionManager> sessionManager;
	IOContextHandler ioContext;

	uint32_t magicNumber;
};