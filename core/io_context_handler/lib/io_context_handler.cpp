//
// Created by guestuser on 06.06.2026.
//
#include "io_context_handler/io_context_handler.h"

IOContextHandler::IOContextHandler() : work_guard(boost::asio::make_work_guard(io_context))
{
	start();
}
IOContextHandler::~IOContextHandler()
{
	stop();
}
void IOContextHandler::start()
{
	thread_work = std::thread([this]() {
		io_context.run();
	});
}
void IOContextHandler::stop()
{
	boost::asio::post(io_context, [this] {
		work_guard.reset();
	});
	if(thread_work.joinable())
		thread_work.join();
}

boost::asio::io_context& IOContextHandler::getIOContext()
{
	return io_context;
}