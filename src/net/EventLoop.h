#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <sys/poll.h>
#include <thread>

#include "TimerQueue.hpp"

class EventLoop {
public:
    using Functor = std::function<void()>;

    EventLoop();

    void loop();

    bool isInLoopThread() const;

    void assertInLoopThread() const;

    void quit();

    void runAt(TimerCallback cb, Timestamp ts);
    void runAfter(TimerCallback cb, std::chrono::seconds);
    void runAfter(TimerCallback cb, std::chrono::milliseconds);
    void runEvery(TimerCallback cb, std::chrono::seconds);
    void runEvery(TimerCallback cb, std::chrono::milliseconds);

    void runInLoop(Functor func);

    ~EventLoop();

private:
    void wakeup();
    void handleRead();

    std::atomic<bool> stop_;
    std::thread::id ownerThreadId_;

    std::unique_ptr<TimerQueue> timerQueue_;
    std::vector<Functor> pendingFunctors_;
    std::mutex mtx_;
    int wakeFd_;

    pollfd wakepollfd_;
};
