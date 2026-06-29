//
// Created by ivan on 12.05.2026.
//
#include "transport_handler/transport_handler.h"
#include "logger/logger.h"
#include "message/transport_message.h"
#include "utils/error_handler.h"
#include <boost/json.hpp>
#include <iostream>

namespace json = boost::json;

void TransportHandler::setErrorMessage(TransportMessage& message)
{
	message.type        = "error";
	message.transaction = Transaction::Error;
	message.payload.clear();
}

TransportHandler::TransportHandler(const std::shared_ptr<tcp::socket>& socket) : socket(socket)
{
	invoker.registerHandler(Command::errorHandler, [this](error_code error, TransportMessage& transportMessage,
	                                                      const std::string& name_function, IOMode mode) {
		if(ErrorHandler::check_error(error, std::string("TransportHandler::read.") +
		                                        (mode == IOMode::Sync ? "sync" : "async") + "{" + name_function +
		                                        "}")) {
			if(onError) {
				onError(error);
			}
		}
		if(error) {
			setErrorMessage(transportMessage);
			if(onAllRead) {
				onAllRead(error, std::move(transportMessage));
			}
			return true;
		}
		return false;
	});
}

void TransportHandler::setMagicNumber(MagicInt mg)
{
	magicNumber = mg;
}

void TransportHandler::setOnReadHandler(const Network::Server::ReadHandler& handler)
{
	onReadHandler = handler;
}

void TransportHandler::setOnWriteHandler(const WriteHandler& handler)
{
	onWriteHandler = handler;
}

void TransportHandler::setOnAllRead(const ErrorAndMessageHandler& handler)
{
	onAllRead = handler;
}

void TransportHandler::setOnAllWrite(const ErrorAndMessageHandler& handler)
{
	onAllWrite = handler;
}

void TransportHandler::read_sync()
{
	TransportMessage transportMessage;
	uint32_t magic, json_len;
	auto bufferSize = sizeof(uint32_t);
	auto buffers    = std::vector{boost::asio::buffer(&magic, bufferSize), boost::asio::buffer(&json_len, bufferSize)};

	error_code error;
	boost::asio::read(*socket, buffers, error);
	if(invoker.invokeHandler(Command::errorHandler, error, transportMessage, "headers", IOMode::Sync))
		return;

	if(magic != magicNumber) {
		Logger::getInstance().log(LogLevel::Error, "TransportHandler::read.sync", "Code = [ Incorrect Magic ]");
		invoker.invokeHandler(Command::errorHandler, boost::asio::error::eof, transportMessage, "headers",
		                      IOMode::Sync);
		return;
	}

	transportMessage.payload.resize(json_len);
	boost::asio::read(*socket, boost::asio::buffer(transportMessage.payload), error);

	if(invoker.invokeHandler(Command::errorHandler, error, transportMessage, "payload", IOMode::Sync))
		return;

	std::string json(reinterpret_cast<const char*>(transportMessage.payload.data()), transportMessage.payload.size());
	json::value json_val = json::parse(json, error);

	if(invoker.invokeHandler(Command::errorHandler, error, transportMessage, "parse_json", IOMode::Sync))
		return;

	if(onReadHandler)
		onReadHandler(0, transportMessage.payload.data(), transportMessage.payload.size() + 1);

	transportMessage.type        = json_val.at("type").as_string();
	transportMessage.transaction = setTypeTransaction(json_val.at("transaction").as_int64());

	if(onAllRead) {
		onAllRead(error, std::move(transportMessage));
	}
}

void TransportHandler::read_async()
{
	auto transportMessage = std::make_shared<TransportMessage>();

	auto magic    = std::make_shared<uint32_t>(0);
	auto json_len = std::make_shared<uint32_t>(0);

	auto bufferSize = sizeof(uint32_t);
	auto buffers =
	    std::vector{boost::asio::buffer(magic.get(), bufferSize), boost::asio::buffer(json_len.get(), bufferSize)};

	boost::asio::async_read(*socket, buffers, [this, magic, json_len, transportMessage](error_code error, size_t) {
		if(invoker.invokeHandler(Command::errorHandler, error, *transportMessage, "headers", IOMode::Async))
			return;
		if(*magic != magicNumber) {
			Logger::getInstance().log(LogLevel::Error, "TransportHandler::read.async", "Code = [ Incorrect Magic ]");
			invoker.invokeHandler(Command::errorHandler, boost::asio::error::eof, *transportMessage, "headers",
			                      IOMode::Async);
			return;
		}

		transportMessage->payload.resize(*json_len);
		boost::asio::async_read(
		    *socket, boost::asio::buffer(transportMessage->payload),
		    [this, transportMessage](error_code error, size_t) {
			    if(invoker.invokeHandler(Command::errorHandler, error, *transportMessage, "payload", IOMode::Async))
				    return;

			    std::string json(reinterpret_cast<const char*>(transportMessage->payload.data()),
			                     transportMessage->payload.size());
			    json::value json_val = json::parse(json, error);

			    if(invoker.invokeHandler(Command::errorHandler, error, *transportMessage, "parse_json", IOMode::Async))
				    return;

			    if(onReadHandler)
				    onReadHandler(0, transportMessage->payload.data(), transportMessage->payload.size());

			    transportMessage->type        = json_val.at("type").as_string();
			    transportMessage->transaction = setTypeTransaction(json_val.at("transaction").as_int64());
			    if(onAllRead) {
				    onAllRead(error, std::move(*transportMessage));
			    }
		    });
	});
}

void TransportHandler::read(IOMode mode)
{
	if(mode == IOMode::Sync) {
		read_sync();
	}
	else {
		read_async();
	}
}

void TransportHandler::write(TransportMessage&& transportMessage, IOMode mode)
{
	auto out_message  = std::make_shared<TransportMessage>(transportMessage);
	uint32_t json_len = out_message->payload.size();
	if(onWriteHandler)
		onWriteHandler(out_message->payload.data(), out_message->payload.size());
	std::vector<boost::asio::const_buffer> buffers;
	buffers.push_back(boost::asio::buffer(&magicNumber, 4));
	buffers.push_back(boost::asio::buffer(&json_len, 4));
	buffers.push_back(boost::asio::buffer(out_message->payload));
	error_code error;
	try {
		if(mode == IOMode::Sync) {
			boost::asio::write(*socket, buffers, error);

			if(ErrorHandler::check_error(error, "TransportHandler::write.sync")) {
				if(onError) {
					onError(error);
				}
			}
			if(onAllWrite) {
				onAllWrite(error, std::move(*out_message));
			}
		}
		else {
			boost::asio::async_write(*socket, buffers, [this, out_message](error_code error, std::size_t) {
				if(ErrorHandler::check_error(error, "TransportHandler::write.async")) {
					if(onError) {
						onError(error);
					}
				}
				if(onAllWrite) {
					onAllWrite(error, std::move(*out_message));
				}
			});
		}
	}
	catch(const std::exception& e) {
		ErrorHandler::check_error(e,
		                          std::string("TransportHandler::write.") + (mode == IOMode::Sync ? "sync" : "async"));
	}
}

void TransportHandler::setOnError(CallbackError handler)
{
	onError = handler;
}