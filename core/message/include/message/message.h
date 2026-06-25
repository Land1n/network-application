//
// Created by ivan on 08.03.2026.
//
#pragma once

#include <string>
#include <boost/json.hpp>
#include "message/transaction.h" // из TransportHandler

namespace json = boost::json;

class Message {
public:
	std::string type;
	Transaction transaction;

	Message(const std::string& type);
	Message(const std::string& type, Transaction transaction);
	virtual json::object serialize();

	virtual ~Message() = default;
};