#ifndef THREAD_SAFE_QUEUE_HPP__
#define THREAD_SAFE_QUEUE_HPP__

#include <condition_variable>
#include <deque>
#include <mutex>

namespace Duty {

template <typename Task>
class ThreadSafeQueue {
public:
    using TaskPtr = std::shared_ptr<Task>;
    using TaskList = std::deque<TaskPtr>;

public:
    ThreadSafeQueue()
        : tasks_ {}
        , mtx_ {}
        , cond_ {}
    {
    }

    void push(Task task)
    {
        TaskPtr ptr = std::make_shared<Task>(std::move(task));
        std::lock_guard<std::mutex> lock(mtx_);
        this->tasks_.push_back(std::move(ptr));
        this->cond_.notify_all();
    }

    void pop(Task& task)
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cond_.wait(lock, [this] { return !this->tasks_.empty(); });
        task = std::move(*tasks_.front());
        tasks_.pop_front();
    }

    TaskPtr pop()
    {
        TaskPtr task { nullptr };
        std::unique_lock<std::mutex> lock(mtx_);
        cond_.wait(lock, [this] { return !this->tasks_.empty(); });
        task = std::move(tasks_.front());
        tasks_.pop_front();
        return task;
    }

    bool try_pop(Task& task)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (this->tasks_.empty())
            return false;
        task = std::move(*this->tasks_.front());
        this->tasks_.pop_front();
        return true;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mtx_);
        return this->tasks_.empty();
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lock(mtx_);
        return this->tasks_.size();
    }

private:
    TaskList tasks_;
    mutable std::mutex mtx_;
    std::condition_variable cond_;
};

template <typename Task>
using ThreadSafeQueuePtr = std::unique_ptr<ThreadSafeQueue<Task>>;

}

#endif