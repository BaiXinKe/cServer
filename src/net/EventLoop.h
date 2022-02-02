#pragma once

#include <atomic>
#include <thread>

class EventLoop {
public:
    EventLoop();

    void loop();

    bool isInLoopThread() const;

    void assertInLoopThread() const;

    void quit();

    ~EventLoop();

private:
    std::atomic<bool> stop_;
    std::thread::id ownerThreadId_;
};