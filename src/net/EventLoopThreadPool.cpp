#include "EventLoopThreadPool.hpp"
#include <assert.h>

namespace Duty {

EventLoopThreadPool::EventLoopThreadPool(uint32_t event_thread_num,
    ThreadInitCallback cb)
    : threadInitCallback_ { cb }
    , event_thread_num_ {
        event_thread_num == DEFAULT_EVENT_THREAD_NUM ? std::thread::hardware_concurrency() : event_thread_num
    }
    , currEventLoopIndex_ { 0 }
{
    assert(event_thread_num_ > 0);

    for (int i = 0; i < event_thread_num_; i++) {
        eventLoopPool_.emplace_back(
            std::move(std::make_unique<EventLoopThread>(threadInitCallback_)));
    }
}

void EventLoopThreadPool::start()
{
    for (auto&& eventLoopThread : eventLoopPool_) {
        eventLoopList_.push_back(eventLoopThread->startLoop());
    }
}

void EventLoopThreadPool::stop()
{
    eventLoopPool_.clear();
    eventLoopList_.clear();
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    this->stop();
}

EventLoop* EventLoopThreadPool::getNextEventLoop()
{
    uint64_t curr = currEventLoopIndex_.fetch_add(1);
    curr %= event_thread_num_;

    return eventLoopList_[curr];
}

}