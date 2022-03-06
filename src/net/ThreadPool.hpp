#ifndef THREAD_POOL_HPP__
#define THREAD_POOL_HPP__

#include <thread>
#include <vector>

namespace Duty {

template <typename Task>
class ThreadPool {
public:
    static constexpr size_t DEFAULT_THREAD_NUM { 0 };
    explicit ThreadPool(size_t initThreadNumber = DEFAULT_THREAD_NUM);

    template <typename ThreadInitCallback>
    std::enable_if_t<std::is_invocable_v<ThreadInitCallback>>
    start(ThreadInitCallback cb = ThreadInitCallback);

private:
    std::vector<std::thread> pool_;
    size_t threadNum_;
};

template <typename Task>
ThreadPool<Task>::ThreadPool(size_t initThreadNumber)
    : threadNum_ { initThreadNumber == DEFAULT_THREAD_NUM ? std::thread::hardware_concurrency() : initThreadNumber }
{
}

template <typename Task>
template <typename ThreadInitCallback>
std::enable_if_t<std::is_invocable_v<ThreadInitCallback>>
ThreadPool<Task>::start(ThreadInitCallback cb)
{
    
}

}

#endif