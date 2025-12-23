#include <request_response_handler.hpp>


MessageData::MessageData(std::string command, std::vector<int> data) : command(command), data(data) {}

MessageData RequestResponseHandler::parseData(const char* data) {
	json::value request_json = json::parse(data);	
	std::cout << request_json << std::endl;
	std::string command = request_json.at("command").as_string().c_str();
	auto& json_array = request_json.at("data").as_array(); 
	std::vector<int> parse_data;	
	for(const auto& item : json_array) 
    		parse_data.push_back(item.as_int64());
	return MessageData(command,parse_data);
}

std::string RequestResponseHandler::serializeData(MessageData data)
{
	json::object response;
	response["command"] = data.command;
	json::array array_data(data.data.begin(),data.data.end());
	response["data"] = array_data;	
	return json::serialize(response);
}


MessageData RequestResponseHandler::processingCommand(const char* data)
{
	MessageData client_md = this->parseData(data);
	if (client_md.command == "getData") {
		return MessageData("sendData",{0});
	}
	else if (client_md.command == "ping") {	
		
		return MessageData("pong",{client_md.data.at(0)+1});
	} else {
		return MessageData("error",{-1});
	}
}
