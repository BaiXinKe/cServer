#pragma once

#include "Timer.hpp"
#include <chrono>
#include <memory>
#include <set>
#include <vector>

class EventLoop;

class TimerQueue {
public:
    using ActivateTimers = std::vector<TimerPtr>;
    using Entry = std::pair<Timestamp, TimerPtr>;

    TimerQueue(EventLoop* loop);
    ~TimerQueue() = default;

    void addTimer(TimerCallback cb, Timestamp timestamp, std::chrono::milliseconds interval);
    void addTimer(TimerCallback cb, Timestamp timestamp, std::chrono::seconds seconds);

    ActivateTimers getExpireation();

    void cancel(TimerId id);
    Timestamp getNextExpirationTimestamp() const;

    void reset(ActivateTimers timers);

private:
    using TimerList = std::set<Entry>;
    using CancelTimer = std::set<TimerId>;

    EventLoop* loop_;
    TimerList timer_list_;
    CancelTimer cancel_timers_;
};