#ifndef _cServer_THREAD_POOL__
#define _cServer_THREAD_POOL__

#include <thread>
#include <future>
#include <vector>
#include <memory>
#include <atomic>
#include <functional>

#include "base/noncopyable.hpp"
#include "base/TaskQueue.hpp"
#include "base/ThreadLocalQueue.hpp"
#include "base/Joiner.hpp"

namespace cServer::base
{
    class function_warpper : noncopyable
    {
        struct impl_base
        {
            virtual void call() = 0;
            virtual ~impl_base() {}
        };
        std::unique_ptr<impl_base> impl;
        template <typename Task>
        struct impl_type : impl_base
        {
            Task task_;
            impl_type(Task &&task)
                : task_{std::move(task)} {}
            void call() { task_(); }
        };

    public:
        function_warpper() = default;

        template <typename Task>
        function_warpper(Task task)
            : impl{new impl_type{std::move(task)}} {}
        void operator()()
        {
            impl->call();
        }

        function_warpper(function_warpper &&other) : impl{std::move(other.impl)}
        {
        }

        function_warpper &operator=(function_warpper &&other)
        {
            impl = std::move(other.impl);
            return *this;
        }
    };

    class ThreadPool : noncopyable
    {
    private:
        using Task = function_warpper;
        std::vector<std::thread> threads_;
        TaskQueue<Task> global_queue_;
        std::vector<ThreadLocalQueue<Task>> local_queues_;
        std::atomic_bool done;
        std::promise<void> start_;
        Joiner thread_joiner;

        static thread_local ThreadLocalQueue<Task> *local_queue_;
        static thread_local size_t index_;

        void run(size_t index);

        inline bool pop_task_from_local_queue(Task &task);

        inline bool pop_task_from_global_queue(Task &task);

        inline bool pop_task_from_other_thread_queue(Task &task);

    public:
        explicit ThreadPool(size_t thread_number = 0);

        ~ThreadPool();
        void run_pending_task();

        template <typename Function, typename... Args>
        std::future<std::invoke_result_t<Function, Args...>>
        submit(Function &&func, Args... args)
        {
            using result_type = std::invoke_result_t<Function, Args...>;
            std::packaged_task<result_type()> task{std::bind(func, args...)};
            std::future<result_type> res{task.get_future()};
            if (this->local_queue_)
                this->local_queue_->push(std::move(task));
            else
                this->global_queue_.push(std::move(task));
            return res;
        }
    };
}

#endif