//
// Created by guestuser on 23.06.2026.
//
#include "message/message.h"

Message::Message(const std::string& type) : type(type), transaction(Transaction::Tests)
{}
Message::Message(const std::string& type, Transaction transaction) : type(type), transaction(transaction)
{}

json::object Message::serialize()
{
	json::object payload;
	payload["type"] = type;
	if(transaction == Transaction::Request)
		payload["transaction"] = 0;
	else if(transaction == Transaction::Response)
		payload["transaction"] = 1;
	else
		payload["transaction"] = -1;
	return payload;
}