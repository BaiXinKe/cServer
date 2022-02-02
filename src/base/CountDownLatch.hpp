#pragma once
#include "noncopyable.hpp"
#include <condition_variable>
#include <mutex>

class CountDownLatch : noncopyable {
public:
    explicit CountDownLatch(int32_t count);

    void done();

    void wait();

    ~CountDownLatch();

private:
    std::mutex mtx_;
    std::condition_variable cond_;
    int32_t count_;
};