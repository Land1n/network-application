//
// Created by ivan on 12.05.2026.
//
#include "TransportHandler/TransportHandler.hpp"

#include "ErrorHandler/ErrorHandler.hpp"

#include <iostream>
#include <bits/codecvt.h>
#include "utils.hpp"

void TransportHandler::setErrorMessage(TransportMessage& message)
{
	message.type        = "error";
	message.transaction = Transaction::Error;
	message.payload.clear();
}

bool isErrorMessage(TransportMessage& message)
{
	return message.type == "error" && message.transaction == Transaction::Error;
}

TransportHandler::TransportHandler(std::shared_ptr<tcp::socket>& socket) : socket(socket)
{}

void TransportHandler::setMagicNumber(uint32_t mg)
{
	magicNumber = mg;
}
void TransportHandler::setOnReadHandler(std::function<void(size_t, const void*, size_t)> handler)
{
	onReadHandler = handler;
}

void TransportHandler::setOnWriteHandler(std::function<void(const void*, size_t)> handler)
{
	onWriteHandler = handler;
}
void TransportHandler::setOnAllRead(const std::function<void(error_code, TransportMessage&&)>& handler)
{
	onAllRead = handler;
}

void TransportHandler::setOnAllWrite(const std::function<void(error_code, TransportMessage&&)>& handler)
{
	onAllWrite = handler;
}

void TransportHandler::read_sync()
{
	TransportMessage transportMessage;
	uint32_t magic, json_len;
	auto bufferSize = sizeof(uint32_t);
	auto buffers    = std::vector{boost::asio::buffer(&magic, bufferSize), boost::asio::buffer(&json_len, bufferSize)};

	auto errorHandler = [this](error_code error, TransportMessage& transportMessage, const std::string& func) {
		if(ErrorHandler::check_error(error, std::string("TransportHandler::read.sync") + "{" + func + "}")) {
			if(onError) {
				onError(error);
			}
		}
		if(error) {
			TransportHandler::setErrorMessage(transportMessage);
			if(onAllRead)
				onAllRead(error, std::move(transportMessage));
			return true;
		}
		return false;
	};

	error_code error;
	boost::asio::read(*socket, buffers, error);
	if(errorHandler(error, transportMessage, "headers"))
		return;

	transportMessage.payload.resize(json_len);
	boost::asio::read(*socket, boost::asio::buffer(transportMessage.payload), error);
	if(errorHandler(error, transportMessage, "payload"))
		return;

	std::string json(reinterpret_cast<const char*>(transportMessage.payload.data()), transportMessage.payload.size());
	json::value json_val = json::parse(json, error);
	if(errorHandler(error, transportMessage, "parse_json"))
		return;

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

	auto errorHandler = [this](error_code error, TransportMessage& transportMessage, const std::string& func) {
		if(ErrorHandler::check_error(error, std::string("TransportHandler::read.async") + "{" + func + "}")) {
			if(onError) {
				onError(error);
			}
		}
		if(error) {
			TransportHandler::setErrorMessage(transportMessage);
			if(onAllRead)
				onAllRead(error, std::move(transportMessage));
			return true;
		}
		return false;
	};

	boost::asio::async_read(
	    *socket, buffers,
	    [this, magic, json_len, transportMessage = std::move(transportMessage),
	     errorHandler = std::move(errorHandler)](error_code error, size_t) {
		    if(errorHandler(error, *transportMessage, "headers"))
			    return;
		    transportMessage->payload.resize(*json_len);
		    boost::asio::async_read(
		        *socket, boost::asio::buffer(transportMessage->payload),
		        [this, transportMessage = std::move(transportMessage),
		         errorHandler = std::move(errorHandler)](error_code error, size_t) {
			        if(errorHandler(error, *transportMessage, "payload"))
				        return;
			        std::string json(reinterpret_cast<const char*>(transportMessage->payload.data()),
			                         transportMessage->payload.size());
			        json::value json_val = json::parse(json, error);
			        if(errorHandler(error, *transportMessage, "parse_json"))
				        return;
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
void TransportHandler::setOnError(std::function<void(error_code)> handler)
{
	onError = handler;
}
