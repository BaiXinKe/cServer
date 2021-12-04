#include "ThreadPool.hpp"

using namespace cServer::base;

thread_local ThreadLocalQueue<ThreadPool::Task> *ThreadPool::local_queue_;
thread_local size_t ThreadPool::index_{};

void ThreadPool::run(size_t index)
{
    this->index_ = index;
    local_queue_ = &local_queues_[this->index_];
    while (!done)
    {
        this->run_pending_task();
    }
}

bool ThreadPool::pop_task_from_local_queue(Task &task)
{
    return this->local_queue_ && this->local_queue_->try_pop(task);
}

bool ThreadPool::pop_task_from_global_queue(Task &task)
{
    return this->global_queue_.try_pop(task);
}

bool ThreadPool::pop_task_from_other_thread_queue(Task &task)
{
    for (size_t i = 0; i < this->local_queues_.size(); ++i)
    {
        unsigned const index = (index_ + 1) % this->local_queues_.size();
        if (this->local_queues_[index].try_steal(task))
            return true;
    }
    return false;
}

ThreadPool::ThreadPool(size_t thread_number) : done{false}, thread_joiner{threads_}
{
    thread_number = (thread_number == 0 ? std::thread::hardware_concurrency() : thread_number);
    this->local_queues_.resize(thread_number);
    try
    {
        for (size_t i = 0; i < thread_number; i++)
        {
            threads_.emplace_back(std::bind(&ThreadPool::run, this, i));
        }
    }
    catch (...)
    {
        done = true;
        throw;
    }
}

ThreadPool::~ThreadPool()
{
    done = true;
}

void ThreadPool::run_pending_task()
{
    Task task;
    if (pop_task_from_local_queue(task) || pop_task_from_global_queue(task) || pop_task_from_other_thread_queue(task))
    {
        task();
    }
    else
    {
        std::this_thread::yield();
    }
}