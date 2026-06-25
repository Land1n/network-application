#pragma once

#include <string>
#include <boost/json.hpp>

namespace json = boost::json;

enum class Configuration {
	Connection,
};

enum class User { Server, Client };

class ConfigurationHandler {
public:
	ConfigurationHandler() = default;

	json::object getData(Configuration config, User user);

protected:
	json::value readFileJson(const std::string& filePath);
	void writeFileJson(const std::string& filePath, const json::value& data);

	json::value createDefaultObject(User user) const;
	std::string getFilePath(Configuration config, User user) const;
};