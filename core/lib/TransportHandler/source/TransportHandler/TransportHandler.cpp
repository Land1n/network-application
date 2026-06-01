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

TransportHandler::TransportHandler(tcp::socket& socket, TypeTransportHandler type) : socket(socket), type(type)
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

void TransportHandler::read(TransportMessage& in_message, boost::system::error_code& code)
{
	if(type == TypeTransportHandler::Sync) {
		sync_read(in_message, code);
	}
	else if(type == TypeTransportHandler::Async) {
		async_read(in_message, code);
	}
}

void TransportHandler::sync_read(TransportMessage& in_message, boost::system::error_code& error)
{
	uint32_t magic, json_len;

	auto buffers = std::vector{boost::asio::buffer(&magic, 4), boost::asio::buffer(&json_len, 4)};
	boost::asio::read(socket, buffers, error);
	ErrorHandler::check_error(error, "TransportHandler::read.sync{buffers}");
	if(error) {
		setErrorMessage(in_message);
	}
	if(!isErrorMessage(in_message)) {
		try {
			in_message.payload.resize(json_len);
			boost::asio::read(socket, boost::asio::buffer(in_message.payload), error);
			ErrorHandler::check_error(error, "TransportHandler::read.sync{payload}");
			if(error) {
				setErrorMessage(in_message);
				return;
			}

			std::string json(reinterpret_cast<const char*>(in_message.payload.data()), in_message.payload.size());
			json::value json_val = json::parse(json, error);
			if(ErrorHandler::check_error(error, "TransportHandler::read.sync{json}")) {
				if(onReadHandler)
					onReadHandler(0, in_message.payload.data(), in_message.payload.size());

				in_message.type        = json_val.at("type").as_string();
				in_message.transaction = setTypeTransaction(json_val.at("transaction").as_int64());
			} else {
				setErrorMessage(in_message);
			}
		}
		catch(const std::exception& e) {
			ErrorHandler::check_error(e, "TransportHandler::read.sync");
			setErrorMessage(in_message);
		}
	}
}

void TransportHandler::async_read(TransportMessage& in_message, boost::system::error_code& error)
{
	auto magic    = std::make_shared<uint32_t>(0);
	auto json_len = std::make_shared<uint32_t>(0);

	auto message_ptr = std::make_shared<TransportMessage>(in_message);

	auto buffers = std::vector{boost::asio::buffer(magic.get(), 4), boost::asio::buffer(json_len.get(), 4)};

	boost::asio::async_read(
	    socket, buffers, [this, magic, json_len, message_ptr, &error](boost::system::error_code code, std::size_t) {
		    ErrorHandler::check_error(code, "TransportHandler::read.async{buffers}");
		    if(!code) {
			    message_ptr->payload.resize(*json_len);
			    boost::asio::async_read(
			        socket, boost::asio::buffer(message_ptr->payload),
			        [this, message_ptr, &error](boost::system::error_code code, std::size_t) {
				        ErrorHandler::check_error(code, "TransportHandler::read.async{payload}");
				        if(!code && !isErrorMessage(*message_ptr)) {
					        try {
					        	std::string json(reinterpret_cast<const char*>(message_ptr->payload.data()), message_ptr->payload.size());
								json::value json_val = json::parse(json, error);
								if(ErrorHandler::check_error(error, "TransportHandler::read.sync{json}")) {
									if(onReadHandler)
										onReadHandler(0, message_ptr->payload.data(), message_ptr->payload.size());

									message_ptr->type        = json_val.at("type").as_string();
									message_ptr->transaction = setTypeTransaction(json_val.at("transaction").as_int64());
								} else {
									setErrorMessage(*
										message_ptr);
								}
					        }
					        catch(const std::exception& e) {
						        ErrorHandler::check_error(e, "TransportHandler::read.async");
						        setErrorMessage(*message_ptr);
					        }
				        }
				        else if(code) {
					        setErrorMessage(*message_ptr);
				        }
				        error = code;
			        });
		    }
		    else {
			    setErrorMessage(*message_ptr);
			    error = code;
		    }
	    });
}

void TransportHandler::write(const TransportMessage& message, boost::system::error_code& error)
{
	uint32_t json_len = message.payload.size();
	if(onWriteHandler)
		onWriteHandler(message.payload.data(), message.payload.size());
	std::vector<boost::asio::const_buffer> buffers;
	buffers.push_back(boost::asio::buffer(&magicNumber, 4));
	buffers.push_back(boost::asio::buffer(&json_len, 4));
	buffers.push_back(boost::asio::buffer(message.payload));

	try {
		if(type == TypeTransportHandler::Sync) {
			boost::asio::write(socket, buffers, error);
			ErrorHandler::check_error(error, "TransportHandler::write.sync");
		}
		else if(type == TypeTransportHandler::Async) {
			boost::asio::async_write(socket, buffers, [](error_code code, std::size_t) {
				ErrorHandler::check_error(code, "TransportHandler::write.async");
			});
		}
	}
	catch(const std::exception& e) {
		ErrorHandler::check_error(e, std::string("TransportHandler::write.") +
		                                 (type == TypeTransportHandler::Sync ? "sync" : "async"));
	}
}
