#include "ConfigurationHandler/ConfigurationHandler.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <filesystem>
#include <ErrorHandler/ErrorHandler.hpp>

static std::string kindToString(boost::json::kind k)
{
	switch(k) {
	case boost::json::kind::null:
		return "null";
	case boost::json::kind::object:
		return "object";
	case boost::json::kind::array:
		return "array";
	case boost::json::kind::string:
		return "string";
	case boost::json::kind::uint64:
		return "uint64";
	case boost::json::kind::int64:
		return "int64";
	case boost::json::kind::double_:
		return "double";
	case boost::json::kind::bool_:
		return "bool";
	default:
		return "unknown";
	}
}

static boost::system::error_code make_boost_error_code(const std::ios_base::failure& ex)
{
	return boost::system::error_code(boost::system::errc::io_error, boost::system::system_category());
}

ConfigurationHandler::ConfigurationHandler() : m_isLoaded(false)
{}

std::string ConfigurationHandler::getFilePath(Configuration config) const
{
	switch(config) {
	case Configuration::Connection:
		return "../config/connection.json";
	default:
		throw std::runtime_error("Unknown configuration type");
	}
}

void ConfigurationHandler::readFileJson(const std::string& filePath)
{
	std::ifstream file(filePath);
	if(!file.is_open()) {
		boost::system::error_code ec(boost::system::errc::no_such_file_or_directory, boost::system::system_category());
		ErrorHandler::check_error(ec, "ConfigurationHandler::readFileJson", true);
		throw std::runtime_error("Cannot open file: " + filePath);
	}

	std::stringstream buffer;
	try {
		buffer << file.rdbuf();
	}
	catch(const std::ios_base::failure& ex) {
		auto ec = make_boost_error_code(ex);
		ErrorHandler::check_error(ec, "ConfigurationHandler::readFileJson", true);
		throw;
	}

	std::string content = buffer.str();
	boost::system::error_code ec;
	json::value data = json::parse(content, ec);
	if(ec) {
		ErrorHandler::check_error(ec, "ConfigurationHandler::readFileJson", true);
		throw std::runtime_error("JSON parse error: " + ec.message());
	}
	m_cachedData = std::move(data);
}

void ConfigurationHandler::writeFileJson(const std::string& filePath, const json::value& data)
{
	// Создаём директорию, если её нет
	std::filesystem::path dir = std::filesystem::path(filePath).parent_path();
	if(!dir.empty() && !std::filesystem::exists(dir)) {
		std::filesystem::create_directories(dir);
	}

	std::string serialized = json::serialize(data);
	std::ofstream file(filePath);
	if(!file.is_open()) {
		boost::system::error_code ec(boost::system::errc::permission_denied, boost::system::system_category());
		ErrorHandler::check_error(ec, "ConfigurationHandler::writeFileJson", true);
		throw std::runtime_error("Cannot open file for writing: " + filePath);
	}

	try {
		file << serialized;
		if(!file.good()) {
			throw std::ios_base::failure("Write failed");
		}
	}
	catch(const std::ios_base::failure& ex) {
		auto ec = make_boost_error_code(ex);
		ErrorHandler::check_error(ec, "ConfigurationHandler::writeFileJson", true);
		throw;
	}
}

json::object ConfigurationHandler::createDefaultServerObject() const
{
	json::object server;
	server["address"]      = "";
	server["port"]         = json::array();
	server["multiConnect"] = json::array();
	return server;
}

json::value ConfigurationHandler::getJson(Configuration config)
{
	std::string filePath = getFilePath(config);
	bool fileLoaded      = false;

	try {
		readFileJson(filePath);
		fileLoaded = true;
	}
	catch(const std::exception& e) {
		ErrorHandler::check_error(e, "ConfigurationHandler::getJson: read failed, will create default");
		fileLoaded = false;
	}

	if(!fileLoaded || m_cachedData.is_null() || !m_cachedData.is_object()) {
		json::object root;
		root["Server"] = createDefaultServerObject();
		root["Client"] = json::object{{"server_address", ""}, {"port_server", json::array()}};
		m_cachedData   = std::move(root);
		try {
			writeFileJson(filePath, m_cachedData);
			ErrorHandler::check_error(boost::system::error_code(),
			                          "ConfigurationHandler::getJson: default config written to " + filePath, true);
		}
		catch(const std::exception& e) {
			ErrorHandler::check_error(e, "ConfigurationHandler::getJson: failed to write default config");
		}
	}

	if(!m_cachedData.is_object()) {
		std::runtime_error ex("JSON root is not an object, type: " + kindToString(m_cachedData.kind()));
		ErrorHandler::check_error(ex, "ConfigurationHandler::getJson");
		throw ex;
	}

	const auto& obj = m_cachedData.as_object();
	switch(config) {
	case Configuration::Connection: {
		auto it = obj.find("Server");
		if(it == obj.end()) {
			std::string errMsg = "Key 'Server' not found even after default creation";
			std::runtime_error ex(errMsg);
			ErrorHandler::check_error(ex, "ConfigurationHandler::getJson");
			throw ex;
		}
		return it->value();
	}
	default:
		std::runtime_error ex("Unknown configuration type");
		ErrorHandler::check_error(ex, "ConfigurationHandler::getJson");
		throw ex;
	}
}