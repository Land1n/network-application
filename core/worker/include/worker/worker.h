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
#include <atomic>

#include "logger/logger.h"

enum class StatusTask { Success = 0, Work = 1, Error = -1 };

struct Task {
	std::function<void()> task;
	StatusTask status;
	Task(std::function<void()> t);
};

enum class StatusWorker { Wait = 0, Work = 1, Flush = 2, Stop = 3 };

class Worker {
public:
	Worker(unsigned short milliseconds = 500);
	Worker(std::queue<std::shared_ptr<Task>> q);

	~Worker();

	void start();
	void stop(bool clearQueue = false);

	void flush();

	StatusWorker const getStatusWorker();
	size_t const getSizeQueue();

	void addTask(std::shared_ptr<Task> task);
	void addTask(std::function<void()> f);

private:
	void runThreadTask();
	void changeStatusWorker(StatusWorker s);

	std::mutex mutex_tasks_data;
	std::condition_variable cv_tasks_data;

	Logger& logger = Logger::getInstance();

	std::queue<std::shared_ptr<Task>> tasks = {};
	std::shared_ptr<Task> current_task      = nullptr;
	std::unique_ptr<std::thread> thread     = nullptr;
	std::atomic<StatusWorker> status        = StatusWorker::Stop;
	unsigned short timeer;

	size_t ID;
};