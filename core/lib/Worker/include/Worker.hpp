//
// Created by ivan on 16.04.2026.
//
#pragma once
#include <queue>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <future>

#include <condition_variable>

#include "../../Logger/include/Logger.hpp"

enum class StatusTask {
    Success = 0,
    Work = 1,
    Error = -1
};

struct Task {
    std::function<void()> task;
    StatusTask status;
    Task(std::function<void()> t) : task(t), status(StatusTask::Work) {}
};

enum class StatusWorker {
    Wait = 0,
    Work = 1,
    Stop = 2
};

class Worker {
public:
    Worker(bool Trace = false);

    ~Worker();
    void addTask(std::shared_ptr<Task> task);

    void start();
    void stop();

    StatusWorker const getStatusWorker();

    size_t const getSizeQueue();

    void addTask(std::function<void()> f);

private:
    void runThreadTask();

    std::mutex mutex_tasks_data;
    std::condition_variable cv_tasks_data;

    Logger& logger = Logger::getInstance();

    std::queue<std::shared_ptr<Task>> tasks;
    std::unique_ptr<std::thread> thread = nullptr;
    std::atomic<StatusWorker> status = StatusWorker::Stop;
};