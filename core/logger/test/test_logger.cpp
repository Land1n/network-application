//
// Created by ivan on 21.04.2026.
//

#include <regex>
#include <gtest/gtest.h>
#include "logger/logger.h"

class CaptureStdout {
public:
	CaptureStdout() : old_buf(std::cout.rdbuf()), ss()
	{
		std::cout.rdbuf(ss.rdbuf());
	}
	~CaptureStdout()
	{
		std::cout.rdbuf(old_buf);
	}
	std::string get() const
	{
		return ss.str();
	}

private:
	std::streambuf* old_buf;
	std::stringstream ss;
};

class LoggerTests : public ::testing::Test {};

TEST_F(LoggerTests, Singleton)
{
	Logger& instance1 = Logger::getInstance();
	Logger& instance2 = Logger::getInstance();
	EXPECT_EQ(&instance1, &instance2);
}

TEST_F(LoggerTests, LogLevelFiltering)
{
	Logger& logger = Logger::getInstance();

	// ---- Проверка порога Debug (1) ----
	{
		CaptureStdout capture;
		logger.setLevel(LogLevel::Debug);

		logger.log(LogLevel::Trace, __func__, "Trace message");       // не выводится (0 < 1)
		logger.log(LogLevel::Debug, __func__, "Debug message");       // выводится (1 >= 1)
		logger.log(LogLevel::Info, __func__, "Info message");         // выводится (2 >= 1)
		logger.log(LogLevel::Warn, __func__, "Warn message");         // выводится (3 >= 1)
		logger.log(LogLevel::Error, __func__, "Error message");       // выводится (4 >= 1)
		logger.log(LogLevel::Critical, __func__, "Critical message"); // выводится (5 >= 1)

		std::string output = capture.get();
		EXPECT_TRUE(output.find("Trace message") == std::string::npos);
		EXPECT_TRUE(output.find("Debug message") != std::string::npos);
		EXPECT_TRUE(output.find("Info message") != std::string::npos);
		EXPECT_TRUE(output.find("Warn message") != std::string::npos);
		EXPECT_TRUE(output.find("Error message") != std::string::npos);
		EXPECT_TRUE(output.find("Critical message") != std::string::npos);
	}

	// ---- Проверка порога Info (2) ----
	{
		CaptureStdout capture;
		logger.setLevel(LogLevel::Info);

		logger.log(LogLevel::Debug, __func__, "Debug2"); // не выводится (1 < 2)
		logger.log(LogLevel::Info, __func__, "Info2");   // выводится (2 >= 2)
		logger.log(LogLevel::Warn, __func__, "Warn2");   // выводится (3 >= 2)

		std::string output = capture.get();
		EXPECT_TRUE(output.find("Debug2") == std::string::npos);
		EXPECT_TRUE(output.find("Info2") != std::string::npos);
		EXPECT_TRUE(output.find("Warn2") != std::string::npos);
	}
}

// Проверка потокобезопасности (без изменений)
TEST_F(LoggerTests, ThreadSafety)
{
	Logger& logger = Logger::getInstance();
	logger.setLevel(LogLevel::Trace);
	const int thread_count  = 3;
	const int message_count = 10;
	std::vector<std::thread> threads;
	std::atomic<int> counter{0};

	for(int i = 1; i <= thread_count; ++i) {
		threads.emplace_back([i = i, &message_count, &logger, &counter]() {
			for(int j = 1; j <= message_count; ++j) {
				logger.log(LogLevel::Info, "thread=" + std::to_string(i), "Message=" + std::to_string(j));
				counter++;
				std::this_thread::sleep_for(std::chrono::milliseconds(10 + i * 10));
			}
		});
	}
	for(auto& t: threads)
		t.join();
	EXPECT_EQ(counter, thread_count * message_count);
}