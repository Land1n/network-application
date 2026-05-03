//
// Created by ivan on 16.04.2026.
//
// #include "Worker.hpp"
#include "../include/Worker.hpp"

#include <iostream>

Task::Task(std::function<void()> t) : task(t), status(StatusTask::Work) {
}

Worker::Worker(unsigned short miliseconds,bool Trace) : timeer(miliseconds) {
    start();
    if (Trace == false) {
        logger.setLevel(LogLevel::Tests);
    }
}

Worker::Worker(std::queue<std::shared_ptr<Task> > q, bool Trace) : tasks(std::move(q)), timeer(500){
    start();
    if (Trace == false) {
        logger.setLevel(LogLevel::Tests);
    }
}

Worker::~Worker() {
    stop(true);
}

void Worker::start() {
    if (status == StatusWorker::Stop) {
        logger.log(LogLevel::Trace, __func__, "Starting worker");
        changeStatusWorker(StatusWorker::Wait);
        thread = std::make_unique<std::thread>([this]() {
            this->runThreadTask();
        });
    }
}

void Worker::stop(bool clearQueue) {
    {
        std::lock_guard<std::mutex> lock(mutex_tasks_data);
        if (status == StatusWorker::Stop) return;
        changeStatusWorker(StatusWorker::Stop);
        if (clearQueue) {
            tasks = std::queue<std::shared_ptr<Task>>();
            current_task.reset();
        }
        cv_tasks_data.notify_all();
    }
    if (thread && thread->joinable() ) {
        thread->join();
        thread = nullptr;
    }

    logger.log(LogLevel::Trace, __func__, "Stopping worker");
}

void Worker::flush() {
    {
        std::lock_guard<std::mutex> lock(mutex_tasks_data);
        if (status == StatusWorker::Flush) return;
        changeStatusWorker(StatusWorker::Flush);
    }
    cv_tasks_data.notify_all();
    {
        std::unique_lock<std::mutex> lock(mutex_tasks_data);
        cv_tasks_data.wait(lock, [this] { return tasks.empty(); });
    }

    if (thread && thread->joinable()) {
        thread->join();
        thread = nullptr;
    }

    logger.log(LogLevel::Trace, __func__, "Flushing worker completed");
}

StatusWorker const Worker::getStatusWorker() {
    std::lock_guard<std::mutex> lock(mutex_tasks_data);
    return status;
}

size_t const Worker::getSizeQueue() {
    std::lock_guard<std::mutex> lock(mutex_tasks_data);
    return tasks.size() + (current_task != nullptr ? 1:0);
}

void Worker::addTask(std::function<void()> f) {
    addTask(std::make_shared<Task>(f));
}

void Worker::addTask(std::shared_ptr<Task> task) {
    std::lock_guard<std::mutex> lock(mutex_tasks_data);
    if (status == StatusWorker::Stop || status == StatusWorker::Flush) {
        return;
    }
    tasks.push(task);
    cv_tasks_data.notify_one();
}

void Worker::runThreadTask() {
    while (status != StatusWorker::Stop) {
        std::unique_lock lock(mutex_tasks_data);
        bool success = cv_tasks_data.wait_for(lock, std::chrono::milliseconds(timeer), [this] {
            return !tasks.empty() || status == StatusWorker::Stop || status == StatusWorker::Flush;
        });
        if (!success) {
            logger.log(LogLevel::Trace, __func__, "Timeer wait_for");
            logger.log(LogLevel::Trace, __func__, std::to_string(tasks.size()));
            if (status == StatusWorker::Stop) break;
            continue;
        }

        if (status == StatusWorker::Stop) break;
        if (status == StatusWorker::Flush && tasks.empty()) break;

        if (!tasks.empty() && status != StatusWorker::Flush) changeStatusWorker(StatusWorker::Work);

        current_task = tasks.front();
        tasks.pop();
        try {
            current_task->task();
            current_task->status = StatusTask::Success;
        } catch (const std::exception &e) {
            current_task->status = StatusTask::Error;
        }
        if (status == StatusWorker::Stop) break;
        current_task.reset();
        if (tasks.empty()) {
            if (status != StatusWorker::Flush)
                changeStatusWorker(StatusWorker::Wait);
            cv_tasks_data.notify_all();
        }
    }
}

void Worker::changeStatusWorker(const StatusWorker s) {
    if (status != s) {
        status = s;
        switch (s) {
            case StatusWorker::Wait: logger.log(LogLevel::Trace, __func__, "StatusWorker = Wait");
                break;
            case StatusWorker::Work: logger.log(LogLevel::Trace, __func__, "StatusWorker = Work");
                break;
            case StatusWorker::Stop: logger.log(LogLevel::Trace, __func__, "StatusWorker = Stop");
                break;
            case StatusWorker::Flush: logger.log(LogLevel::Trace, __func__, "StatusWorker = Flush");
                break;
        }
    }
}