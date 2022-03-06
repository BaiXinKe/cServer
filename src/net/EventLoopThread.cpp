#include "EventLoopThread.hpp"
#include "EventLoop.hpp"

namespace Duty {

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb)
    : loop_ { nullptr }
    , exiting_ { false }
    , thread_ { [this] { this->threadFunction(); } }
    , callback_(cb)
    , start_pms_ {}
    , start_fut_ {}
{
    start_fut_ = start_pms_.get_future();
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
    EventLoop* loop = nullptr;
    {
        start_pms_.set_value();

        std::unique_lock<std::mutex> lock(mtx_);
        while (loop_ == nullptr) {
            cond_.wait(lock);
        }

        loop = loop_;
    }

    return loop_;
}

void EventLoopThread::threadFunction()
{
    start_fut_.wait();

    EventLoop loop;
    if (callback_) {
        callback_(&loop);
    }

    {
        std::lock_guard<std::mutex> lock(mtx_);
        loop_ = &loop;
        cond_.notify_all();
    }

    loop.loop();

    std::lock_guard<std::mutex> lock(mtx_);
    loop_ = nullptr;
}

}
