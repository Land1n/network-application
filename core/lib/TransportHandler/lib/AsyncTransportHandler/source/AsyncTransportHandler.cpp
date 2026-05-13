//
// Created by ivan on 12.05.2026.
//
#include "AsyncTransportHandler.hpp"

#include <memory>

AsyncTransportHandler::AsyncTransportHandler(std::shared_ptr<tcp::socket> socket, uint32_t magicNumber) :
    BaseTransportHandler(socket, magicNumber)
{}

void AsyncTransportHandler::read(TransportMessage& message, boost::system::error_code& error)
{
	uint32_t magic = 0;

	auto error_handler = [&message, &error](boost::system::error_code code, std::size_t) {
		if(code) {
			message.type        = "error";
			message.transaction = Transaction::Error;
			error               = code;
		}
	};
	// while(magic != magicNumber) {
	boost::asio::async_read(*socket, boost::asio::buffer(&magic, 4), error_handler);
	if(message.type == "error" || message.transaction == Transaction::Error) {
		return;
	}
	// }

	uint32_t json_len;
	boost::asio::async_read(*socket, boost::asio::buffer(&json_len, 4), error_handler);

	if(message.type == "error" || message.transaction == Transaction::Error) {
		return;
	}
	message.payload.resize(json_len);
	boost::asio::async_read(*socket, boost::asio::buffer(message.payload), error_handler);

	if(message.type == "error" || message.transaction == Transaction::Error) {
		message.type        = "error";
		message.transaction = Transaction::Error;
		message.payload.clear();
		return;
	}
	if(message.type != "error" && message.transaction != Transaction::Error) {
		try {
			std::string_view json(reinterpret_cast<const char*>(message.payload.data()), message.payload.size());
			json::value json_val = json::parse(json);
			if(onReadHandler)
				onReadHandler(0, message.payload.data(), message.payload.size());
			// пока не предумал как делать
			message.type        = json_val.at("type").as_string();
			message.transaction = setTypeTransaction(json_val.at("transaction").as_int64());
			logger.log(LogLevel::Info, __func__, "Read TransportMessage");
		}
		catch(const std::exception& e) {
			message.type        = "error";
			message.transaction = Transaction::Error;
			logger.log(LogLevel::Critical, __func__, "Read TransportMessage invalid msg");
			logger.log(LogLevel::Critical, __func__, e.what());
			return;
		}
		logger.log(LogLevel::Info, __func__, "Request TransportMessage.type: " + message.type);
	}
	else {
		logger.log(LogLevel::Error, __func__, "Request TransportMessage.type: " + message.type);
		logger.log(LogLevel::Error, __func__, error.what());
	}
}

void AsyncTransportHandler::write(const TransportMessage& message, boost::system::error_code& error)
{
	uint32_t json_len = message.payload.size();
	logger.log(LogLevel::Info, __func__, "Response TransportMessage.type: " + message.type);

	if(onWriteHandler)
		onWriteHandler(message.payload.data(), message.payload.size());
	std::vector<boost::asio::const_buffer> buffers;
	buffers.push_back(boost::asio::buffer(&magicNumber, 4));
	buffers.push_back(boost::asio::buffer(&json_len, 4));
	buffers.push_back(boost::asio::buffer(message.payload));
	try {
		boost::asio::async_write(*socket, buffers, [this](boost::system::error_code code, std::size_t) {
			if(!code)
				logger.log(LogLevel::Info, __func__, "Write response TransportMessage");
			else
				logger.log(LogLevel::Warn, __func__, "Write response TransportMessage failed");
		});
	}
	catch(const std::exception& e) {
		logger.log(LogLevel::Critical, __func__, e.what());
	}
}