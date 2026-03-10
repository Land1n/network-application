#pragma once

#include <boost/asio.hpp>
#include <boost/thread.hpp>

class ThreadPool {
public:

    explicit ThreadPool(size_t pool_size);
    ~ThreadPool();

    template <typename Task>
    void enqueue(Task task) {
        io_context.post(task);
    }

private:
    boost::asio::io_context io_context;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work;
    boost::thread_group threads;
    size_t pool_size_;   
};