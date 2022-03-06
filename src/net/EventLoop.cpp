#include "EventLoop.hpp"

#include <cassert>
#include <iostream>
#include <thread>

#include <spdlog/spdlog.h>

thread_local std::thread::id local_thread_id_;

Duty::EventLoop::EventLoop()
    : stop_ { false }
    , runningPending_ { false }
    , poll_ { std::make_unique<Poller>() }
    , timerTaskQueue_ { std::make_unique<TimerQueue>() }
    , pendingTask_ { std::make_unique<ThreadSafeQueue<PendingTask>>() }
{
    assert(stop_ == false);
    local_thread_id_ = std::this_thread::get_id();
    wake_.reset(new WakeUp(this));
}

void Duty::EventLoop::assertInLoopThread() const
{
    assert(isInLoopThread());
}

bool Duty::EventLoop::isInLoopThread() const
{
    return local_thread_id_ == std::this_thread::get_id();
}

Duty::EventLoop::~EventLoop()
{
    stop_ = true;
}

void Duty::EventLoop::loop()
{
    assertInLoopThread();
    while (!stop_) {
        activedChannels_.clear();
        auto ts = timerTaskQueue_->getNextExpiredTime();

        poll_->poll(activedChannels_, ts);

        doPendingTasks();
        for (auto&& actived : activedChannels_) {
            actived->handleEvent();
        }
        activedChannels_.clear();

        timerTaskQueue_->getExpiredTimes(expired_timers_);
        execExpiredTimesTask();
    }
}

void Duty::EventLoop::update(Channel* channel)
{
    assertInLoopThread();
    this->poll_->update(channel);
}

void Duty::EventLoop::removeChannel(Channel* channel)
{
    this->assertInLoopThread();
    this->poll_->removeChannel(channel);
}

void Duty::EventLoop::runInLoop(PendingTask task)
{
    if (isInLoopThread()) {
        task();
    } else {
        pendingTask_->push(std::move(task));
        wake_->handleWrite();
    }
}

void Duty::EventLoop::runAfter(TimerCallback timercb, std::chrono::milliseconds millseconds)
{
    PendingTask task { [this, timercb, millseconds] {
        timerTaskQueue_->runAfter(std::move(timercb), millseconds);
    } };
    this->runInLoop(std::move(task));
}
void Duty::EventLoop::runAfter(TimerCallback timercb, double seconds)
{
    this->runInLoop([this, timercb, seconds] {
        timerTaskQueue_->runAfter(std::move(timercb), seconds);
    });
}

void Duty::EventLoop::runUntil(TimerCallback timercb, Timestamp expire_time)
{
    this->runInLoop([this, timercb, expire_time] {
        timerTaskQueue_->runUntil(std::move(timercb), expire_time);
    });
}
void Duty::EventLoop::runUntil(TimerCallback timercb, time_t expire_time)
{
    this->runInLoop([this, timercb, expire_time] {
        timerTaskQueue_->runUntil(std::move(timercb), expire_time);
    });
}

void Duty::EventLoop::runEvery(TimerCallback timercb, std::chrono::milliseconds millseconds)
{
    this->runInLoop([this, timercb, millseconds] {
        timerTaskQueue_->runEvery(std::move(timercb), millseconds);
    });
}
void Duty::EventLoop::runEvery(TimerCallback timercb, double seconds)
{
    this->runInLoop([this, timercb, seconds] {
        timerTaskQueue_->runEvery(std::move(timercb), seconds);
    });
}

void Duty::EventLoop::execExpiredTimesTask()
{
    assertInLoopThread();
    for (auto& timer : expired_timers_) {
        timer->run();
        if (timer->repeat()) {
            timerTaskQueue_->addTimer(std::move(timer));
        }
    }
    expired_timers_.clear();
}

void Duty::EventLoop::doPendingTasks()
{
    assertInLoopThread();

    runningPending_ = true;

    PendingTask task;
    while (!pendingTask_->empty()) {
        std::cout << "CallOnce" << std::endl;
        pendingTask_->pop(task);
        task();
    }

    runningPending_ = false;
}