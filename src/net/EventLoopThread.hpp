#ifndef EVENT_LOOP_THREAD_HPP__
#define EVENT_LOOP_THREAD_HPP__

#include <condition_variable>
#include <mutex>
#include <thread>

#include <future>

#include <functional>

namespace Duty {

class EventLoop;

class EventLoopThread {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback());
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void threadFunction();

private:
    std::promise<void> start_pms_;
    std::future<void> start_fut_;

    EventLoop* loop_;
    bool exiting_;
    std::thread thread_;
    std::mutex mtx_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};

}

#endif