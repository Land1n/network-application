//
// Created by ivan on 16.04.2026.
//
// #include "Worker.hpp"
#include "../include/Worker.hpp"

#include <iostream>

Task::Task(std::function<void()> t) : task(t), status(StatusTask::Work) {
}

/// + TODO: stop vs flush
/// stop - стопит и не дожидается выполнения ВСЕХ задач в очереди
/// flush - целиком отрабатывает очередь, потом join thread
/// проверка - во время flush таски нельзя добавлять
/// ? Чистить ли очередь после stop
Worker::Worker(bool Trace) {
    start();
    if (Trace == false) {
        logger.setLevel(LogLevel::Tests);
    }
}

Worker::Worker(std::queue<std::shared_ptr<Task> > q, bool Trace) {
    tasks = q;
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
        if (clearQueue) tasks = std::queue<std::shared_ptr<Task> >();
        if (status == StatusWorker::Stop) return;
        changeStatusWorker(StatusWorker::Stop);
    }
    cv_tasks_data.notify_all();

    if (thread && thread->joinable())
        thread->join();

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
    // Дожидаемся, пока поток обработает все задачи и завершится
    if (thread && thread->joinable())
        thread->join();

    logger.log(LogLevel::Trace, __func__, "Flushing worker completed");
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
    if (status == StatusWorker::Stop || status == StatusWorker::Flush) {
        return;
    }
    tasks.push(task);
    cv_tasks_data.notify_one();
}

void Worker::runThreadTask() {
    while (status != StatusWorker::Stop) {
        std::unique_lock<std::mutex> lock(mutex_tasks_data);
        bool success = cv_tasks_data.wait_for(lock, std::chrono::milliseconds(500), [this] {
            return !tasks.empty() || status == StatusWorker::Stop || status == StatusWorker::Flush;
        });
        if (!success) {
            logger.log(LogLevel::Trace, __func__, "Timeer wait_for");
            logger.log(LogLevel::Trace, __func__, std::to_string(tasks.size()));
            continue;
        }

        if (status == StatusWorker::Stop) break;
        else if (status == StatusWorker::Flush && tasks.empty()) break;

        if (!tasks.empty() && status != StatusWorker::Flush) changeStatusWorker(StatusWorker::Work);

        auto task = tasks.front();
        try {
            task->task();
            task->status = StatusTask::Success;
        } catch (const std::exception &e) {
            task->status = StatusTask::Error;
            break;
        }
        tasks.pop();
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
