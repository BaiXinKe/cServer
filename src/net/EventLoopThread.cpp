#include "EventLoopThread.hpp"
#include "EventLoop.hpp"

namespace Duty {

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb)
    : start_flags_ {}
    , wait_falgs_ { start_flags_.get_future() }
    , loop_ { nullptr }
    , exiting_ { false }
    , thread_ { [this] { this->threadFunction(); } }
    , callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr) {
        loop_->stop();
        if (thread_.joinable())
            thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    start_flags_.set_value();
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cond_.wait(lock, [this] { return this->loop_ != nullptr; });
    }
    return loop_;
}

void EventLoopThread::threadFunction()
{
    wait_falgs_.wait();

    EventLoop loop;
    if (callback_) {
        callback_(&loop);
    }

    {
        std::lock_guard<std::mutex> lock(mtx_);
        this->loop_ = &loop;
        cond_.notify_all();
    }

    loop.loop();

    std::lock_guard<std::mutex> lock(mtx_);
    loop_ = nullptr;
}

}