#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <boost/json.hpp>

#include "configuration_handler/configuration_handler.h"

namespace fs   = std::filesystem;
namespace json = boost::json;

class ConfigurationHandlerIntegrationTest : public ::testing::Test {
protected:
	fs::path testRoot;
	fs::path originalCwd;
	ConfigurationHandler handler;

	void SetUp() override
	{
		originalCwd = fs::current_path();

		auto temp           = fs::temp_directory_path() / "cfg_test_XXXXXX";
		std::string tempStr = temp.string();
		if(mkdtemp(tempStr.data()) == nullptr) {
			throw std::runtime_error("Cannot create temp dir");
		}
		testRoot = tempStr;

		fs::create_directories(testRoot / "temp_test");
		fs::create_directories(testRoot / "config");

		fs::current_path(testRoot / "temp_test");
	}

	void TearDown() override
	{
		fs::current_path(originalCwd);
		fs::remove_all(testRoot);
	}

	void writeConfigFile(const std::string& relPath, const json::value& data)
	{
		fs::path fullPath = fs::current_path() / relPath;
		fs::create_directories(fullPath.parent_path());
		std::ofstream file(fullPath);
		file << json::serialize(data);
	}

	std::string readFileContent(const std::string& relPath)
	{
		fs::path fullPath = fs::current_path() / relPath;
		std::ifstream file(fullPath);
		std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		return content;
	}
};

// Тест 1: при отсутствии файла создаётся дефолтный конфиг
TEST_F(ConfigurationHandlerIntegrationTest, CreatesDefaultConfigIfMissing)
{
	auto serverData     = handler.getData(Configuration::Connection, User::Server);
	fs::path serverPath = fs::current_path() / "../config/server_connection.json";
	EXPECT_TRUE(fs::exists(serverPath));
	EXPECT_TRUE(serverData.contains("ports"));
	EXPECT_TRUE(serverData.contains("multiConnect"));
	EXPECT_TRUE(serverData.contains("magic"));
	EXPECT_EQ(serverData.at("ports").as_array().size(), 1);
	EXPECT_EQ(serverData.at("ports").as_array()[0].as_int64(), 10000);

	auto clientData     = handler.getData(Configuration::Connection, User::Client);
	fs::path clientPath = fs::current_path() / "../config/client_connection.json";
	EXPECT_TRUE(fs::exists(clientPath));
	EXPECT_TRUE(clientData.contains("address"));
	EXPECT_TRUE(clientData.contains("port"));
	EXPECT_TRUE(clientData.contains("magic"));
	EXPECT_EQ(clientData.at("address").as_string(), "127.0.0.1");
	EXPECT_EQ(clientData.at("port").as_int64(), 10000);
}

TEST_F(ConfigurationHandlerIntegrationTest, ReadsValidConfig)
{
	json::value serverConfig = {{"ports", {8080, 8081}}, {"multiConnect", {true, false}}, {"magic", {42, 43}}};
	json::value clientConfig = {{"address", "192.168.1.1"}, {"port", 9000}, {"magic", 123}};
	writeConfigFile("../config/server_connection.json", serverConfig);
	writeConfigFile("../config/client_connection.json", clientConfig);

	auto serverData = handler.getData(Configuration::Connection, User::Server);
	auto clientData = handler.getData(Configuration::Connection, User::Client);

	EXPECT_EQ(serverData.at("ports").as_array().size(), 2);
	EXPECT_EQ(serverData.at("ports").as_array()[0].as_int64(), 8080);
	EXPECT_EQ(serverData.at("ports").as_array()[1].as_int64(), 8081);
	EXPECT_EQ(serverData.at("multiConnect").as_array()[0].as_bool(), true);
	EXPECT_EQ(serverData.at("magic").as_array()[1].as_int64(), 43);

	EXPECT_EQ(clientData.at("address").as_string(), "192.168.1.1");
	EXPECT_EQ(clientData.at("port").as_int64(), 9000);
	EXPECT_EQ(clientData.at("magic").as_int64(), 123);
}

TEST_F(ConfigurationHandlerIntegrationTest, RecoversFromCorruptedJson)
{
	fs::path serverPath = fs::current_path() / "../config/server_connection.json";
	fs::create_directories(serverPath.parent_path());
	std::ofstream file(serverPath);
	file << "{ this is not json";
	file.close();

	auto serverData = handler.getData(Configuration::Connection, User::Server);
	EXPECT_TRUE(serverData.contains("ports"));
	EXPECT_EQ(serverData.at("ports").as_array().size(), 1);

	std::string content = readFileContent("../config/server_connection.json");
	EXPECT_NO_THROW(json::parse(content));
}

TEST_F(ConfigurationHandlerIntegrationTest, ThrowsOnMissingUser)
{
	json::value clientOnly = {{"Client", {{"address", "1.2.3.4"}, {"port", 1234}, {"magic", 567}}}};
	writeConfigFile("../config/server_connection.json", clientOnly);

	EXPECT_THROW({ handler.getData(Configuration::Connection, User::Server); }, std::runtime_error);
}