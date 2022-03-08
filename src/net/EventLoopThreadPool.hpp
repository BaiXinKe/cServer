#ifndef EVENT_LOOP_THREAD_POOL_HPP__
#define EVENT_LOOP_THREAD_POOL_HPP__

#include "EventLoopThread.hpp"
#include <atomic>
#include <vector>

namespace Duty {

using ThreadInitCallback = std::function<void(EventLoop*)>;

class EventLoopThreadPool {
public:
    static constexpr uint32_t DEFAULT_EVENT_THREAD_NUM { 0 };
    explicit EventLoopThreadPool(uint32_t event_thread_num = DEFAULT_EVENT_THREAD_NUM,
        ThreadInitCallback cb = ThreadInitCallback());

    void start();

    void stop();

    EventLoop* getNextEventLoop();

    ~EventLoopThreadPool();

private:
    std::vector<std::unique_ptr<EventLoopThread>> eventLoopPool_;
    std::vector<EventLoop*> eventLoopList_;

    ThreadInitCallback threadInitCallback_;

    uint32_t event_thread_num_;
    std::atomic<uint64_t> currEventLoopIndex_;
};

}

#endif