#include "ConfigurationHandler/ConfigurationHandler.hpp"

#include <fstream>
#include <filesystem>
#include <stdexcept>

namespace fs   = std::filesystem;
namespace json = boost::json;

ConfigurationHandler::ConfigurationHandler() = default;

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

json::value ConfigurationHandler::createDefaultObject() const
{
	return json::value{{"Server", json::object{{"port", json::array()}, {"multiConnect", json::array()}}},
	                   {"Client", json::object{{"server_address", ""}, {"port_server", json::array()}}}};
}

std::string ConfigurationHandler::getFilePath(Configuration config, User /*user*/) const
{
	switch(config) {
	case Configuration::Connection:
		return "../config/connection.json";
	default:
		throw std::invalid_argument("Unknown configuration type");
	}
}

json::object ConfigurationHandler::getData(Configuration config, User user)
{
	std::string filePath = getFilePath(config, user);
	json::value root;

	try {
		root = readFileJson(filePath);
	}
	catch(const std::exception&) {
		root = createDefaultObject();
		writeFileJson(filePath, root);
	}

	if(!root.is_object()) {
		root = createDefaultObject();
		writeFileJson(filePath, root);
	}

	const json::object& rootObj = root.as_object();

	switch(user) {
	case User::Server:
		if(rootObj.contains("Server") && rootObj.at("Server").is_object()) {
			return rootObj.at("Server").as_object();
		}
		break;
	case User::Client:
		if(rootObj.contains("Client") && rootObj.at("Client").is_object()) {
			return rootObj.at("Client").as_object();
		}
		break;
	}

	throw std::runtime_error("Invalid configuration structure for user");
}