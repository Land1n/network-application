//
// Created by ivan on 21.04.2026.
//

#include <regex>
#include <gtest/gtest.h>

#include "Logger.hpp"

// + TODO: допилить (потоки с разной задержкой)
class CaptureStdout {
public:
    CaptureStdout() : old_buf(std::cout.rdbuf()), ss() {
        std::cout.rdbuf(ss.rdbuf());
    }
    ~CaptureStdout() {
        std::cout.rdbuf(old_buf);
    }
    std::string get() const { return ss.str(); }
private:
    std::streambuf* old_buf;
    std::stringstream ss;
};


class LoggerTests : public ::testing::Test {};

TEST_F(LoggerTests, Singleton) {
    Logger& instance1 = Logger::getInstance();
    Logger& instance2 = Logger::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(LoggerTests, LogLevelFiltering) {
    Logger& logger = Logger::getInstance();
    CaptureStdout capture;

    // Уровень Info – должны выводиться только Info и Tests (числовые значения <=1)
    logger.setLevel(LogLevel::Info);
    logger.log(LogLevel::Debug, __func__, "Debug message");   // не выводится (5 <= 1 ? false)
    logger.log(LogLevel::Info, __func__, "Info message");     // выводится (1 <= 1 true)
    logger.log(LogLevel::Tests, __func__, "Tests message");   // выводится (0 <= 1 true)

    std::string output = capture.get();
    EXPECT_TRUE(output.find("Debug message") == std::string::npos);
    EXPECT_TRUE(output.find("Info message") != std::string::npos);
    EXPECT_TRUE(output.find("Tests message") != std::string::npos);
}

// Дополнительно: проверка потокобезопасности (базовая)
TEST_F(LoggerTests, ThreadSafety) {
    Logger& logger = Logger::getInstance();
    logger.setLevel(LogLevel::Trace);
    const int thread_count = 3;
    const int message_count = 10;
    std::vector<std::thread> threads;
    std::atomic<int> counter{0};

    for (int i = 1; i <= thread_count; ++i) {

        threads.emplace_back([i = i,&message_count,&logger, &counter]() {
            for (int j = 1; j <= message_count; ++j) {
                logger.log(LogLevel::Info, "thread="+std::to_string(i), "Message=" + std::to_string(j));
                counter++;
                std::this_thread::sleep_for(std::chrono::milliseconds(10+i*10));
            }
        });
    }
    for (auto& t : threads) t.join();
    EXPECT_EQ(counter, thread_count * message_count);
    // Если программа не упала и не было гонок данных – тест пройден
}