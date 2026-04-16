//
// Created by ivan on 16.04.2026.
//
// #include "Worker.hpp"
#include "../include/Worker.hpp"

#include <boost/asio/detail/mutex.hpp>

Worker::Worker(bool Trace) {
    start();
    if (Trace == false) {
        logger.setLevel(LogLevel::Tests);
    }
}

Worker::~Worker() {
    stop();
}

void Worker::start() {
    if (status == StatusWorker::Stop) {
        status = StatusWorker::Wait;
        logger.log(LogLevel::Trace, __func__, "Starting worker");
        thread = std::make_unique<std::thread>([this]() {
            this->runThreadTask();
        });
    }
}

void Worker::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_tasks_data);
        if (status == StatusWorker::Stop) return;
        status = StatusWorker::Stop;
    }
    cv_tasks_data.notify_all();

    if (thread && thread->joinable())
        thread->join();

    logger.log(LogLevel::Trace, __func__, "Stopping worker");
}

StatusWorker const Worker::getStatusWorker() {
    std::lock_guard<std::mutex> lock(mutex_tasks_data);
    return status;
}

size_t const Worker::getSizeQueue() {
    std::lock_guard<std::mutex> lock(mutex_tasks_data);
    return tasks.size();
}

void Worker::addTask(std::function<void()> f) {
    addTask(std::make_shared<Task>(f));
}

void Worker::addTask(std::shared_ptr<Task> task) {
    std::lock_guard<std::mutex> lock(mutex_tasks_data);
    if (status == StatusWorker::Stop) {
        return;
    }
    tasks.push(task);
    cv_tasks_data.notify_one();
}

void Worker::runThreadTask() {
    while (status != StatusWorker::Stop) {
        std::unique_lock<std::mutex> lock(mutex_tasks_data);
        bool success = cv_tasks_data.wait_for(lock, std::chrono::milliseconds(200), [this] {
            return !tasks.empty() || status == StatusWorker::Stop;
        });
        if (!success) {
            break;
        }
        if (status == StatusWorker::Stop && tasks.empty()) {
            break;
        }
        if (!tasks.empty())
            status = StatusWorker::Work;
        auto task = tasks.front();
        try {
            logger.log(LogLevel::Trace, __func__, "Status task = " + std::to_string(static_cast<int>(task->status)));
            task->task();
            task->status = StatusTask::Success;
        } catch (const std::exception &e) {
            task->status = StatusTask::Error;
            break;
        }
        tasks.pop();
        lock.unlock();
        logger.log(LogLevel::Trace, __func__, "Status task = " + std::to_string(static_cast<int>(task->status)));
        if (tasks.empty())
            status = StatusWorker::Wait;
    }
}
