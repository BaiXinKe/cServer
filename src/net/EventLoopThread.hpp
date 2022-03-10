#ifndef EVENTLOOP_THREAD_HPP__
#define EVENTLOOP_THREAD_HPP__

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <thread>

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
    std::promise<void> start_flags_;
    std::future<void> wait_falgs_;

    EventLoop* loop_;
    bool exiting_;
    std::mutex mtx_;
    std::condition_variable cond_;

    std::thread thread_;
    ThreadInitCallback callback_;
};

}

#endif