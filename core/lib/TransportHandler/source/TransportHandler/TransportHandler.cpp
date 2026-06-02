//
// Created by ivan on 12.05.2026.
//
#include "TransportHandler/TransportHandler.hpp"

#include "ErrorHandler/ErrorHandler.hpp"

#include <iostream>
#include <bits/codecvt.h>

void setErrorMessage(TransportMessage& message)
{
	message.type        = "error";
	message.transaction = Transaction::Error;
	message.payload.clear();
}

bool isErrorMessage(TransportMessage& message)
{
	return message.type == "error" && message.transaction == Transaction::Error;
}

TransportHandler::TransportHandler(tcp::socket& socket) : socket(socket)
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
void TransportHandler::setOnAllRead(const std::function<void(error_code,TransportMessage)>& handler)
{
	onAllRead = handler;
}

void TransportHandler::setOnAllWrite(const std::function<void(error_code,TransportMessage)>& handler)
{
	onAllWrite = handler;
}

std::future<error_code> TransportHandler::read(TransportMessage& in_message, IOMode mode)
{
	if(mode == IOMode::Sync) {
		return sync_read(in_message);
	}
	return async_read(in_message);
}

std::future<error_code> TransportHandler::sync_read(TransportMessage& in_message)
{
	uint32_t magic, json_len;
	error_code error;
	std::promise<error_code> promise;
	auto buffers = std::vector{boost::asio::buffer(&magic, 4), boost::asio::buffer(&json_len, 4)};
	boost::asio::read(socket, buffers, error);
	ErrorHandler::check_error(error, "TransportHandler::read.sync{buffers}");
	if(error) {
		setErrorMessage(in_message);
		promise.set_value(error);
	}
	if(!isErrorMessage(in_message)) {
		in_message.payload.resize(json_len);
		boost::asio::read(socket, boost::asio::buffer(in_message.payload), error);
		ErrorHandler::check_error(error, "TransportHandler::read.sync{payload}");
		if(error) {
			setErrorMessage(in_message);
			promise.set_value(error);
		}

		std::string json(reinterpret_cast<const char*>(in_message.payload.data()), in_message.payload.size());
		json::value json_val = json::parse(json, error);
		if(ErrorHandler::check_error(error, "TransportHandler::read.sync{json}")) {
			if(onReadHandler)
				onReadHandler(0, in_message.payload.data(), in_message.payload.size());
			try {
				in_message.type        = json_val.at("type").as_string();
				in_message.transaction = setTypeTransaction(json_val.at("transaction").as_int64());
				promise.set_value({});
				if(onAllRead) {
					onAllRead(error,in_message);
				}
			}
			catch(const std::exception& e) {
				ErrorHandler::check_error(e, "TransportHandler::read.sync");
				setErrorMessage(in_message);
				return promise.get_future();
			}
		}
		else {
			setErrorMessage(in_message);
			promise.set_value(error);
		}
	}
	return promise.get_future();
}

std::future<error_code> TransportHandler::async_read(TransportMessage& in_message)
{
	auto magic    = std::make_shared<uint32_t>(0);
	auto json_len = std::make_shared<uint32_t>(0);

	auto bufferSize = sizeof(uint32_t);
	auto buffers =
	    std::vector{boost::asio::buffer(magic.get(), bufferSize), boost::asio::buffer(json_len.get(), bufferSize)};

	auto promise = std::make_shared<std::promise<error_code>>();

	boost::asio::async_read(
	    socket, buffers, [this, magic, json_len, &in_message, promise](error_code error, std::size_t) {
		    ErrorHandler::check_error(error, "TransportHandler::read.async{buffers}");
		    if(!error) {
			    in_message.payload.resize(*json_len);
			    boost::asio::async_read(
			        socket, boost::asio::buffer(in_message.payload),
			        [this, &in_message, promise](boost::system::error_code error, std::size_t) {
				        ErrorHandler::check_error(error, "TransportHandler::read.async{payload}");
				        if(!error && !isErrorMessage(in_message)) {
					        try {
						        std::string json(reinterpret_cast<const char*>(in_message.payload.data()),
						                         in_message.payload.size());
						        json::value json_val = json::parse(json, error);
						        if(ErrorHandler::check_error(error, "TransportHandler::read.sync{json}")) {
							        if(onReadHandler)
								        onReadHandler(0, in_message.payload.data(), in_message.payload.size());
							        in_message.type        = json_val.at("type").as_string();
							        in_message.transaction = setTypeTransaction(json_val.at("transaction").as_int64());
							        promise->set_value({});
							        if(onAllRead) {
								        onAllRead(error,in_message);
							        }
						        }
						        else {
							        setErrorMessage(in_message);
							        promise->set_value(error);
						        }
					        }
					        catch(const std::exception& e) {
						        ErrorHandler::check_error(e, "TransportHandler::read.async");
						        setErrorMessage(in_message);
						        promise->set_value(error);
					        }
				        }
				        else if(error) {
					        setErrorMessage(in_message);
					        promise->set_value(error);
				        }
			        });
		    }
		    else {
			    setErrorMessage(in_message);
			    promise->set_value(error);
		    }
	    });
	return promise->get_future();
}

std::future<error_code> TransportHandler::write(const TransportMessage& message, IOMode mode)
{
	uint32_t json_len = message.payload.size();
	if(onWriteHandler)
		onWriteHandler(message.payload.data(), message.payload.size());
	std::vector<boost::asio::const_buffer> buffers;
	buffers.push_back(boost::asio::buffer(&magicNumber, 4));
	buffers.push_back(boost::asio::buffer(&json_len, 4));
	buffers.push_back(boost::asio::buffer(message.payload));
	auto promise = std::make_shared<std::promise<error_code>>();
	error_code error;
	try {
		if(mode == IOMode::Sync) {
			boost::asio::write(socket, buffers, error);
			promise->set_value(error);
			ErrorHandler::check_error(error, "TransportHandler::write.sync");
		}
		else if(mode == IOMode::Async) {
			boost::asio::async_write(socket, buffers, [promise](error_code code, std::size_t) {
				promise->set_value(code);
				ErrorHandler::check_error(code, "TransportHandler::write.async");
			});
		}
		if(onAllWrite) {
			onAllWrite(error,message);
		}
	}
	catch(const std::exception& e) {
		ErrorHandler::check_error(e, std::string("TransportHandler::write.") + getSyncOrAsync(mode));
	}
	return promise->get_future();
}
