#ifndef THREAD_POOL_HPP__
#define THREAD_POOL_HPP__

#include <thread>
#include <vector>

#include <functional>
#include <future>

#include "ThreadSafeQueue.hpp"

namespace Duty {

using Task = std::function<void()>;
using ThreadPoolRunningFunction = std::function<void(ThreadSafeQueuePtr<Task>& taskQueue,
    const std::atomic<bool>& stop)>;

class ThreadPool {
public:
    static constexpr uint32_t DEFAULT_THREAD_POOL_SIZE { 0 };

    explicit ThreadPool(ThreadPoolRunningFunction running,
        uint32_t thread_number = DEFAULT_THREAD_POOL_SIZE);

    template <typename Func, typename... Args>
    auto submit(Func&& func, Args... args)
        -> std::future<std::invoke_result_t<Func, Args...>>;

    void start();

    void stop();

    ~ThreadPool();

private:
    std::vector<std::thread> pool_;
    uint32_t thread_number_;
    ThreadSafeQueuePtr<Task> tasks_;
    std::atomic<bool> stop_;

    ThreadPoolRunningFunction runningFunction_;
};

template <typename Func, typename... Args>
auto ThreadPool::submit(Func&& func, Args... args)
    -> std::future<std::invoke_result_t<Func, Args...>>
{
    using result_type = std::invoke_result_t<Func, Args...>;
    auto packaged { std::make_shared<std::packaged_task<result_type()>>(
        std::bind(std::forward<Func>(func), std::forward<Args>(args)...)) };

    auto result_fut = packaged->get_future();
    tasks_->push([packaged] {
        (*packaged)();
    });

    return result_fut;
}

}

#endif