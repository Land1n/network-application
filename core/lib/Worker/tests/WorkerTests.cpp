//
// Created by ivan on 16.04.2026.
//
#include <gtest/gtest.h>
#include <future>
#include "Worker.hpp"

/// TODO: main.cpp в тестах
/// + TODO: класс-родитель для тестов
class WorkerTests : public ::testing::Test {
public:
    Worker worker;

    void TearDown() override {
        worker.stop(true);
    }
};

TEST_F(WorkerTests, addOneTaskAndStop) {
    unsigned short i = 0;
    auto task = std::make_shared<Task>([&i]() {
        for (; i < 3; ++i);
    });
    worker.addTask(task);
    while (task->status == StatusTask::Work) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    EXPECT_EQ(i, 3);
    EXPECT_EQ(task->status, StatusTask::Success);
    worker.stop();
}

TEST_F(WorkerTests, addOneTaskAndFlush) {
    unsigned short i = 0;
    auto task = std::make_shared<Task>([&i]() {
        for (; i < 3; ++i);
    });
    worker.addTask(task);
    worker.flush();
    EXPECT_EQ(i, 3);
    EXPECT_EQ(task->status, StatusTask::Success);
}

TEST_F(WorkerTests, addNTask) {
    unsigned int N = 10;
    std::vector<std::shared_ptr<Task> > tasks;
    for (int i = 0; i < N; ++i) {
        unsigned short counter = i;
        auto task = std::make_shared<Task>([&counter]() {
            counter += 1;
        });
        tasks.push_back(task);
        worker.addTask(task);
    }
    worker.flush();
    for (auto task: tasks) {
        EXPECT_EQ(task->status, StatusTask::Success);
    }
}

TEST_F(WorkerTests, taskThrowsException) {
    auto task = std::make_shared<Task>([]() {
        throw std::runtime_error("test error");
    });
    worker.addTask(task);
    // Даём время выполниться
    while (task->status == StatusTask::Work) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    EXPECT_EQ(task->status, StatusTask::Error);
}

/// + TODO: cv.wait....
TEST_F(WorkerTests, twoWorkersIndependent) {
    Worker w1, w2;
    std::atomic<int> counter1{0}, counter2{0};

    for (int i = 0; i < 10; ++i) {
        w1.addTask([&counter1]() { counter1++; });
        w2.addTask([&counter2]() { counter2++; });
    }
    // То что было
    // while (w1.getSizeQueue() != 0 || w2.getSizeQueue() != 0) {
    //     std::this_thread::sleep_for(std::chrono::milliseconds(1));
    // }
    // То что стало
    w1.flush();
    w2.flush();
    EXPECT_EQ(counter1.load(), 10);
    EXPECT_EQ(counter2.load(), 10);
}

TEST_F(WorkerTests, StopFalseDoTask) {
    std::atomic<bool> started{false};

    worker.addTask([&started]() {
        started.store(true);
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    });
    if (started.load())
        worker.stop(false);
    EXPECT_EQ(worker.getSizeQueue(), 1);
}


TEST_F(WorkerTests, StopTrueDoTask) {
    worker.addTask([]() { std::this_thread::sleep_for(std::chrono::microseconds(500)); });

    worker.stop(true);
    EXPECT_EQ(worker.getSizeQueue(), 0);
}

TEST_F(WorkerTests, FlushDoTask) {
    // Logger::getInstance().setLevel(LogLevel::Trace);
    worker.addTask([]() { std::this_thread::sleep_for(std::chrono::microseconds(500)); });
    worker.flush();
    EXPECT_EQ(worker.getSizeQueue(), 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    if (!argv)
        ::testing::FLAGS_gtest_filter = "WorkerTests.*";

    return RUN_ALL_TESTS();
}
