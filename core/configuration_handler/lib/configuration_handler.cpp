#include "configuration_handler/configuration_handler.h"

#include <fstream>
#include <filesystem>
#include <stdexcept>

namespace fs   = std::filesystem;
namespace json = boost::json;

json::value ConfigurationHandler::readFileJson(const std::string& filePath)
{
	std::ifstream file(filePath);
	if(!file.is_open()) {
		throw std::runtime_error("Cannot open file: " + filePath);
	}

	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	return json::parse(content);
}

void ConfigurationHandler::writeFileJson(const std::string& filePath, const json::value& data)
{
	fs::path path(filePath);
	fs::path parent = path.parent_path();
	if(!parent.empty() && !fs::exists(parent)) {
		fs::create_directories(parent);
	}

	std::ofstream file(filePath);
	if(!file.is_open()) {
		throw std::runtime_error("Cannot write to file: " + filePath);
	}

	file << json::serialize(data);
	file.close();
}

json::value ConfigurationHandler::createDefaultObject(User user) const
{
	switch(user) {
	case User::Server:
		return json::object{
		    {"ports", json::array{10000}}, {"multiConnect", json::array{false}}, {"magic", json::array{123456}}};
	case User::Client:
		return json::object{{"address", "127.0.0.1"}, {"port", 10000}, {"magic", 123456}};
	default:
		throw std::invalid_argument("Unknown user");
	}
}

std::string ConfigurationHandler::getFilePath(Configuration config, User user) const
{
	try {
		return configMap.at({user, config});
	}
	catch(const std::exception& e) {
		throw std::invalid_argument("Unknown type");
	}
}

ConfigurationHandler::ConfigurationHandler()
{
	configMap[{User::Server, Configuration::Connection}] = "../config/server_connection.json";
	configMap[{User::Client, Configuration::Connection}] = "../config/client_connection.json";
}
void ConfigurationHandler::setPath(Configuration config, User user, std::string path)
{
	configMap[{user, config}] = path;
}
json::object ConfigurationHandler::getData(Configuration config, User user)
{
	std::string filePath = getFilePath(config, user);
	json::value root;

	try {
		root = readFileJson(filePath);
	}
	catch(const std::exception&) {
		root = createDefaultObject(user);
		writeFileJson(filePath, root);
	}

	if(!root.is_object()) {
		root = createDefaultObject(user);
		writeFileJson(filePath, root);
	}

	const auto& obj = root.as_object();
	bool valid      = true;

	if(user == User::Server) {
		if(!obj.contains("ports") || !obj.at("ports").is_array() || !obj.contains("multiConnect") ||
		   !obj.at("multiConnect").is_array() || !obj.contains("magic") || !obj.at("magic").is_array()) {
			valid = false;
		}
	}
	else if(user == User::Client) {
		if(!obj.contains("address") || !obj.at("address").is_string() || !obj.contains("port") ||
		   !obj.at("port").is_int64() || !obj.contains("magic") || !obj.at("magic").is_int64()) {
			valid = false;
		}
	}

	if(!valid) {
		throw std::runtime_error("Invalid configuration structure for user");
	}

	return obj;
}

