#ifndef _cServer_TASKQUEUE__
#define _cServer_TASKQUEUE__

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

#include "base/noncopyable.hpp"

namespace cServer::base
{
    template <typename Task>
    class TaskQueue : noncopyable
    {
    private:
        std::queue<std::shared_ptr<Task>> tasks_;
        mutable std::mutex mtx_;
        std::condition_variable cond_;

    public:
        TaskQueue() : tasks_{} {}

        TaskQueue(TaskQueue &&other)
        {
            std::lock_guard<std::mutex> lock_(other.mtx_);
            this->tasks_.swap(other.tasks_);
        }

        TaskQueue &operator=(TaskQueue &&other) = delete;

        void push(Task task)
        {
            std::shared_ptr<Task> task_{std::make_shared<Task>(std::move(task))};
            std::lock_guard<std::mutex> lock_(this->mtx_);
            tasks_.push(std::move(task_));
            cond_.notify_one();
        }

        bool try_pop(Task &task_)
        {
            std::lock_guard<std::mutex> lock_(this->mtx_);
            if (this->tasks_.empty())
                return false;
            task_ = std::move(*this->tasks_.front());
            this->tasks_.pop();
        }

        std::shared_ptr<Task> try_pop()
        {
            std::lock_guard<std::mutex> lock_(this->mtx_);
            if (this->tasks_.empty())
                return std::shared_ptr<Task>();
            std::shared_ptr<Task> task{std::move(this->tasks_.front())};
            this->tasks_.pop();
            return task;
        }

        void wait_pop(Task &task_)
        {
            std::unique_lock<std::mutex> lock_(this->mtx_);
            cond_.wait(lock_, [&]
                       { return !this->tasks_.empty(); });
            task_ = std::move(*this->tasks_.front());
            this->tasks_.pop();
        }

        std::shared_ptr<Task> wait_pop()
        {
            std::unique_lock<std::mutex> lock_(this->mtx_);
            cond_.wait(lock_, [&]
                       { return !this->tasks_.empty(); });
            std::shared_ptr<Task> task{std::move(this->tasks_.front())};
            this->tasks_.pop();
            return task;
        }

        bool empty() const
        {
            std::lock_guard<std::mutex> lock_(this->mtx_);
            return this->tasks_.empty();
        }

        ~TaskQueue() {}
    };
}

#endif