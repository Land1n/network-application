#pragma once

#include <map>
#include <string>
#include <boost/json.hpp>

namespace json = boost::json;

enum class Configuration {
	Connection,
};

enum class User { Server, Client };

class ConfigurationHandler {
public:
	ConfigurationHandler();

	void setPath(Configuration config, User user, std::string path);

	json::object getData(Configuration config, User user);

protected:
	json::value readFileJson(const std::string& filePath);
	void writeFileJson(const std::string& filePath, const json::value& data);

	json::value createDefaultObject(User user) const;
	std::string getFilePath(Configuration config, User user) const;

private:
	std::map<std::pair<User, Configuration>, std::string> configMap;
};