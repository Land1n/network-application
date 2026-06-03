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

TransportHandler::TransportHandler(tcp::socket& socket) : socket(socket),onError(onError)
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
void TransportHandler::setOnAllRead(const std::function<void(error_code, TransportMessage)>& handler)
{
	onAllRead = handler;
}

void TransportHandler::setOnAllWrite(const std::function<void(error_code, TransportMessage)>& handler)
{
	onAllWrite = handler;
}

void TransportHandler::read(TransportMessage& in_message, IOMode mode)
{
	error_code error;

	auto magic    = std::make_shared<uint32_t>(0);
	auto json_len = std::make_shared<uint32_t>(0);

	auto bufferSize = sizeof(uint32_t);
	auto buffers = std::vector{boost::asio::buffer(magic.get(), bufferSize), boost::asio::buffer(json_len.get(), bufferSize)};

	auto short_print = [&mode](error_code ec, const std::string& read_what) {
		ErrorHandler::check_error(ec, std::string("TransportHandler::read.") + getSyncOrAsync(mode) + "{" + read_what + "}");
	};

	if(mode == IOMode::Sync) {


		boost::asio::read(socket, buffers, error);
		short_print(error, "buffer");
		if(error) {
			setErrorMessage(in_message);
			if(onAllRead) onAllRead(error, in_message);
			return;
		}


		in_message.payload.resize(*json_len);
		boost::asio::read(socket, boost::asio::buffer(in_message.payload), error);
		short_print(error, "payload");
		if(error) {
			setErrorMessage(in_message);
			if(onAllRead) onAllRead(error, in_message);
			return;
		}


		std::string json(reinterpret_cast<const char*>(in_message.payload.data()), in_message.payload.size());
		json::value json_val = json::parse(json, error);
		short_print(error, "json");
		if(error) {
			if(onAllRead) onAllRead(error, in_message);
			setErrorMessage(in_message);
			return;
		}


		in_message.type        = json_val.at("type").as_string();
		in_message.transaction = setTypeTransaction(json_val.at("transaction").as_int64());
		if(onAllRead) onAllRead(error, in_message);
	}
	else {
		boost::asio::async_read(
		    socket, buffers,
		    [this, magic, json_len, &in_message, short_print](error_code ec, size_t) {
			    short_print(ec, "buffer");
			    if(ec) {
				    setErrorMessage(in_message);
			    	if (onAllRead) onAllRead(ec,in_message);
				    return;
			    }


			    in_message.payload.resize(*json_len);
			    boost::asio::async_read(socket, boost::asio::buffer(in_message.payload), [&](error_code ec, size_t) {
				    short_print(ec, "payload");
				    if(ec) {
					    setErrorMessage(in_message);
				    	if (onAllRead) onAllRead(ec,in_message);
					    return;
				    }


				    std::string json(reinterpret_cast<const char*>(in_message.payload.data()),
				                     in_message.payload.size());
				    json::value json_val = json::parse(json, ec);
			    	short_print(ec, "json");
					if(ec) {
						setErrorMessage(in_message);
						if (onAllRead) onAllRead(ec,in_message);
						return;
					}
					in_message.type        = json_val.at("type").as_string();
					in_message.transaction = setTypeTransaction(json_val.at("transaction").as_int64());
			    	if (onAllRead) onAllRead(ec,in_message);
			    });
		    });
	}
}

void TransportHandler::write(const TransportMessage& message, IOMode mode)
{
	uint32_t json_len = message.payload.size();
	if(onWriteHandler)
		onWriteHandler(message.payload.data(), message.payload.size());
	std::vector<boost::asio::const_buffer> buffers;
	buffers.push_back(boost::asio::buffer(&magicNumber, 4));
	buffers.push_back(boost::asio::buffer(&json_len, 4));
	buffers.push_back(boost::asio::buffer(message.payload));
	error_code error;
	try {
		if(mode == IOMode::Sync) {
			boost::asio::write(socket, buffers, error);
			if(ErrorHandler::check_error(error, "TransportHandler::write.sync")) {
				if (onError) onError();
			}
			if(onAllWrite) {
				onAllWrite(error, message);
			}
		}
		else {
			boost::asio::async_write(socket, buffers, [](error_code code, std::size_t) {
				ErrorHandler::check_error(code, "TransportHandler::write.async");
			});
			if(onAllWrite) {
				onAllWrite(error, message);
			}
		}
	}
	catch(const std::exception& e) {
		ErrorHandler::check_error(e, std::string("TransportHandler::write.") + getSyncOrAsync(mode));
	}
}
void TransportHandler::setOnError(std::function<void()> handler)
{
	onError = handler;
}

