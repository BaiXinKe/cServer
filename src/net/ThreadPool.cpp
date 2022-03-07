#include "ThreadPool.hpp"
#include <assert.h>

namespace Duty {

ThreadPool::ThreadPool(ThreadPoolRunningFunction running, uint32_t thread_num)
    : thread_number_ { thread_num == DEFAULT_THREAD_POOL_SIZE ? std::thread::hardware_concurrency() : thread_num }
    , tasks_ { std::make_unique<ThreadSafeQueue<Task>>() }
    , stop_ { false }
    , runningFunction_ { running }
{
    assert(thread_number_ > 0);
}

void ThreadPool::start()
{
    for (uint32_t i = 0; i < thread_number_; i++) {
        pool_.emplace_back(this->runningFunction_, std::ref(this->tasks_), std::cref(this->stop_));
    }
}

void ThreadPool::stop()
{
    stop_ = true;
    for (uint32_t i = 0; i < thread_number_; i++) {
        this->submit([] {});
    }
}

ThreadPool::~ThreadPool()
{
    this->stop();
    for (auto& thread : pool_) {
        if (thread.joinable())
            thread.join();
    }
}

}