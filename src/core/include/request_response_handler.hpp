#pragma once


#include <iostream>
#include <boost/json.hpp>
#include <vector> 

namespace json = boost::json;

struct MessageData
{
	std::string command;
	std::vector<int> data;
	MessageData(std::string command, std::vector<int>data);
};

class RequestResponseHandler
{
public:
	MessageData parseData(const char* data);
	std::string serializeData(MessageData data);
	MessageData processingCommand(const char* data);
};
