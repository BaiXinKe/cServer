#include "EventLoop.h"
#include <cassert>
#include <poll.h>
#include <spdlog/spdlog.h>
#include <sys/eventfd.h>

EventLoop::EventLoop()
    : stop_ { false }
    , ownerThreadId_ { std::this_thread::get_id() }
    , timerQueue_ { std::make_unique<TimerQueue>(this) }
    , wakeFd_ { eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK) }
{
    wakepollfd_.fd = wakeFd_;
    wakepollfd_.events = POLLIN;
}

void EventLoop::loop()
{
    assertInLoopThread();
    while (!stop_) {
        Timestamp ts = timerQueue_->getNextExpirationTimestamp();
        Timestamp now = std::chrono::system_clock::now();

        int ms = std::chrono::duration_cast<std::chrono::milliseconds>(ts - now).count();
        ::poll(&wakepollfd_, 1, ms);

        auto timerTask = timerQueue_->getExpireation();
        for (auto& task : timerTask) {
            task->run();
        }

        timerQueue_->reset(std::move(timerTask));
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

void EventLoop::runAt(TimerCallback cb, Timestamp ts)
{
    timerQueue_->addTimer(cb, ts, std::chrono::milliseconds());
}

void EventLoop::runAfter(TimerCallback cb, std::chrono::seconds secs)
{
    runAfter(cb, std::chrono::duration_cast<std::chrono::milliseconds>(secs));
}

void EventLoop::runAfter(TimerCallback cb, std::chrono::milliseconds ms)
{
    if (ms == std::chrono::milliseconds(0)) {
        runInLoop(cb);
        return;
    }
    Timestamp now = std::chrono::system_clock::now();
    now += ms;
    timerQueue_->addTimer(cb, now, std::chrono::milliseconds(0));
}

void EventLoop::runEvery(TimerCallback cb, std::chrono::seconds secs)
{
    runEvery(cb, std::chrono::duration_cast<std::chrono::milliseconds>(secs));
}

void EventLoop::runEvery(TimerCallback cb, std::chrono::milliseconds ms)
{
    if (ms == std::chrono::milliseconds(0)) {
        spdlog::critical("The time interval of runEveny is too small");
        exit(EXIT_FAILURE);
    }

    Timestamp now = std::chrono::system_clock::now();
    now += ms;
    timerQueue_->addTimer(cb, now, ms);
}

void EventLoop::runInLoop(Functor func)
{
    if (isInLoopThread()) {
        func();
    } else {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            pendingFunctors_.push_back(std::move(func));
        }
        wakeup();
    }
}

void EventLoop::wakeup()
{
    int64_t wake { 1 };
    if (write(wakeFd_, &wake, sizeof(int64_t)) != sizeof(int64_t)) {
        spdlog::critical("write to wakeFd_ error");
        exit(EXIT_FAILURE);
    }
}

void EventLoop::handleRead()
{
    int64_t resetRead;
    if (read(wakeFd_, &resetRead, sizeof(int64_t)) != sizeof(int64_t)) {
        if (errno != EAGAIN || errno != EWOULDBLOCK) {
            spdlog::critical("partition/failed read from wakeFd_");
            exit(EXIT_FAILURE);
        }
    }
}