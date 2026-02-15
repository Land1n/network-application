#include "thread_pool.hpp"


ThreadPool::ThreadPool(size_t pool_size)
    : io_context(),
      work(boost::asio::make_work_guard(io_context)), 
      pool_size_(pool_size)
{
    for (size_t i = 0; i < pool_size_; ++i) {
        threads.create_thread([this]() { io_context.run(); });
    }
}

ThreadPool::~ThreadPool() {
    work.reset();      
    io_context.stop(); 
    threads.join_all();
}