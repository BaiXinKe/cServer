#ifndef _cServer_THREADLOCAL_QUEUE__
#define _cServer_THREADLOCAL_QUEUE__

#include <deque>
#include <memory>
#include <mutex>

#include "base/noncopyable.hpp"

namespace cServer::base
{
    template <typename Task>
    class ThreadLocalQueue : noncopyable
    {
    private:
        std::deque<std::shared_ptr<Task>> tasks_;
        mutable std::mutex mtx_;

    public:
        ThreadLocalQueue() = default;

        ThreadLocalQueue(ThreadLocalQueue &&other)
        {
            std::lock_guard<std::mutex> lock_(other.mtx_);
            tasks_.swap(other.tasks_);
        }

        ThreadLocalQueue &operator=(ThreadLocalQueue &&other) = delete;

        void push(Task task)
        {
            std::shared_ptr<Task> task_{std::make_shared<Task>(std::move(task))};
            std::lock_guard<std::mutex> lock_(this->mtx_);
            this->tasks_.push_front(std::move(task_));
        }

        bool try_pop(Task &task)
        {
            std::lock_guard<std::mutex> lock_(this->mtx_);
            if (this->tasks_.empty())
                return false;
            task = std::move(*this->tasks_.front());
            this->tasks_.pop_front();
            return true;
        }

        std::shared_ptr<Task> try_pop()
        {
            std::lock_guard<std::mutex> lock_(this->mtx_);
            if (this->tasks_.empty())
                return std::shared_ptr<Task>();
            std::shared_ptr<Task> res{this->tasks_.front()};
            this->tasks_.pop_front();
            return res;
        }

        bool try_steal(Task &task)
        {
            std::lock_guard<std::mutex> lock_(this->mtx_);
            if (this->tasks_.empty())
                return false;
            task = std::move(*this->tasks_.back());
            this->tasks_.pop_back();
            return true;
        }

        std::shared_ptr<Task> try_steal()
        {
            std::lock_guard<std::mutex> lock_(this->mtx_);
            if (this->tasks_.empty())
                return std::shared_ptr<Task>();
            std::shared_ptr<Task> res{this->tasks_.back()};
            this->tasks_.pop_back();
            return res;
        }

        bool empty() const
        {
            std::lock_guard<std::mutex> lock_(this->mtx_);
            return this->tasks_.empty();
        }
    };
}

#endif