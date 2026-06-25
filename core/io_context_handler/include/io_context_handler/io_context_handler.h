//
// Created by guestuser on 06.06.2026.
//
#pragma once

#include <boost/asio.hpp>

enum class IOMode {
	Sync  = 0,
	Async = 1,
};

class IOContextHandler {
public:
	IOContextHandler();
	~IOContextHandler();

	boost::asio::io_context& getIOContext();
	void start();
	void stop();

protected:
	boost::asio::io_context io_context;
	std::thread thread_work;
	boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard;
};