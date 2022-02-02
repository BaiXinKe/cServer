#include "CountDownLatch.hpp"

CountDownLatch::CountDownLatch(int32_t count)
    : count_ { count }
{
}

void CountDownLatch::done()
{
    std::lock_guard<std::mutex> lock(mtx_);
    count_--;
    if (count_ == 0)
        cond_.notify_all();
}

void CountDownLatch::wait()
{
    std::unique_lock<std::mutex> lock(mtx_);
    cond_.wait(lock, [&] { return this->count_ == 0; });
}
