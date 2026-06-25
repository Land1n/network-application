//
// Created by ivan on 14.03.2026.
//
#pragma once

#include "clientserveriface/client.h"
#include "session/session.h"

#include <memory>

#include "worker/worker.h"

#include "utils/alias.h"

class SegmentClient : public Network::Client {
public:
	SegmentClient(const std::string& address, PortInt port);
	~SegmentClient() override;

	void start() override;
	void stop() override;

	void connect();
	void disconnect() override;

	void write(const void* data, size_t sz) override;

	void setIOMode(IOMode mode);
	void setMagicNumber(MagicInt magicNumber);

	bool getIsWork();

private:
	std::string address;
	int port;

	IOMode mode = IOMode::Sync;

	std::atomic<bool> isWork{false};
	Worker mainWorker;
	std::unique_ptr<Session> session;
	IOContextHandler ioContext;
	MagicInt magicNumber;
};