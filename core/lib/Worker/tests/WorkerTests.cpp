//
// Created by ivan on 16.04.2026.
//
#include <gtest/gtest.h>
#include <future>
#include "Worker.hpp"

bool all(std::vector<std::shared_ptr<Task>> tasks) {
    int i = 0;
    for (auto task : tasks) {
        i+=1;
        if (task->status == StatusTask::Work) {
            std::cout << i << std::endl;
            return false;
        }
    }
    return true;
}

GTEST_TEST(WorkerTests, addOneTask) {
    Worker worker;

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

GTEST_TEST(WorkerTests, addNTask) {
    Worker worker;

    unsigned int N = 100;
    std::vector<std::shared_ptr<Task>> tasks;
    for (int i = 0; i < N; ++i) {
        unsigned short counter = i;
        auto task = std::make_shared<Task>([&counter]() {
            counter += 1;
        });
        tasks.push_back(task);
        worker.addTask(task);
    }
    while (worker.getSizeQueue() != 0) // && all(tasks) == true)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::cout << " " << std::endl;
    for (auto task : tasks) {
        EXPECT_EQ(task->status, StatusTask::Success);
    }
    worker.stop();
}
