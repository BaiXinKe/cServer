#include "TimerQueue.hpp"

static constexpr int MILL_PER_SECONDS { 1000 };

bool Duty::TimerQueue::TimerPtrLess::operator()(const TimerPtr& left, const TimerPtr& right) const
{
    return (*left) < (*right);
}

void Duty::TimerQueue::runAfter(TimerCallback timercb, std::chrono::milliseconds millsconds)
{
    Timestamp ts = now() + millsconds;
    runUntil(std::move(timercb), std::move(ts));
}

void Duty::TimerQueue::runAfter(TimerCallback timercb, double seconds)
{
    uint64_t millseconds = seconds * MILL_PER_SECONDS;
    Timestamp ts = now() + std::chrono::milliseconds(millseconds);
    runUntil(std::move(timercb), std::move(ts));
}

void Duty::TimerQueue::runUntil(TimerCallback timercb, Timestamp expire_time)
{
    TimerPtr timer { std::make_unique<Timer>(std::move(timercb), expire_time) };

    assert(timer->repeat() == false);
    assert(timer->interval() == std::chrono::milliseconds());

    timers_.insert(std::move(timer));
}

void Duty::TimerQueue::runUntil(TimerCallback timercb, time_t expire_time)
{
    Timestamp ts { std::chrono::seconds(expire_time) };
    runUntil(std::move(timercb), ts);
}

void Duty::TimerQueue::runEvery(TimerCallback Timercb, std::chrono::milliseconds millseconds)
{
    Timestamp expire_time = now() + millseconds;
    TimerPtr timer { std::make_unique<Timer>(std::move(Timercb), expire_time, millseconds) };

    assert(timer->repeat());
    assert(timer->interval() == millseconds);

    timers_.insert(std::move(timer));
}

void Duty::TimerQueue::runEvery(TimerCallback timercb, double seconds)
{
    std::chrono::milliseconds ms {
        std::chrono::milliseconds(static_cast<uint64_t>(seconds * MILL_PER_SECONDS))
    };
    runEvery(std::move(timercb), ms);
}

Duty::Timestamp Duty::TimerQueue::getNextExpiredTime() const
{
    if (timers_.empty()) {
        return Timestamp {};
    }
    auto next = timers_.begin();
    return (*next)->expTime();
}

void Duty::TimerQueue::getExpiredTimes(std::vector<TimerPtr>& expired_times)
{
    Timestamp curr { now() };

    while (!timers_.empty() && (*timers_.begin())->expTime() <= curr) {
        auto timer_node = timers_.extract(timers_.begin());
        expired_times.push_back(std::move(timer_node.value()));
    }
}

void Duty::TimerQueue::addTimer(TimerPtr timer)
{
    assert(timer != nullptr);
    assert(timers_.find(timer) == timers_.end());

    timer->restart();
    timers_.insert(std::move(timer));
}