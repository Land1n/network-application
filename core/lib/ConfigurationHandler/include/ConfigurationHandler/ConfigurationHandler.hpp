#pragma once
#include <string>
#include <boost/json.hpp>

namespace json = boost::json;

enum class Configuration {
	Connection = 0,
};

class ConfigurationHandler {
public:
	ConfigurationHandler();

	json::value getJson(Configuration config);

protected:
	void readFileJson(const std::string& filePath);
	void writeFileJson(const std::string& filePath, const json::value& data);
	json::object createDefaultServerObject() const;
	std::string getFilePath(Configuration config) const;

private:
	json::value m_cachedData;
	Configuration m_currentConfig;
	bool m_isLoaded;
};