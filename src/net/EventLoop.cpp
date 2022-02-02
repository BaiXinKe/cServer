#include "EventLoop.h"
#include <cassert>

EventLoop::EventLoop()
    : stop_ { false }
    , ownerThreadId_ { std::this_thread::get_id() }
{
}

void EventLoop::loop()
{
    assertInLoopThread();
    while (!stop_) {
    }
}

void EventLoop::quit()
{
    stop_ = true;
}

EventLoop::~EventLoop()
{
    quit();
}

bool EventLoop::isInLoopThread() const
{
    return ownerThreadId_ == std::this_thread::get_id();
}

void EventLoop::assertInLoopThread() const
{
    assert(isInLoopThread());
}
