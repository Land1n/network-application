//
// Created by ivan on 14.03.2026.
//
#pragma once

#include "../../clientserveriface/include/clientserveriface/client.h"
#include "Session/Session.hpp"

#include <memory>

class SegmentClient : public Network::Client {
public:
	SegmentClient(const std::string& address, int port);
	~SegmentClient() override;

	void start() override;
	void stop() override;

	void connect();
	void disconnect() override;

	void write(const void* data, size_t sz) override;

	void setIOMode(IOMode mode);

	bool getIsWork();

private:
	std::string address;
	int port;

	IOMode mode = IOMode::Sync;

	std::atomic<bool> isWork{false};
	std::thread mainThread;
	std::unique_ptr<Session> session;
	IOContextHandler ioContext;
};