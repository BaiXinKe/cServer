#ifndef TIMERQUEUE_HPP__
#define TIMERQUEUE_HPP__

#include "Callbacks.hpp"
#include "Timer.hpp"

#include <memory>
#include <set>

namespace Duty {

class TimerQueue {
public:
    TimerQueue() = default;

    void runAfter(TimerCallback timercb, std::chrono::milliseconds millseconds);
    void runAfter(TimerCallback timercb, double seconds);

    void runUntil(TimerCallback timercb, Timestamp expire_time);
    void runUntil(TimerCallback timercb, time_t expire_time);

    void runEvery(TimerCallback timercb, std::chrono::milliseconds millseconds);
    void runEvery(TimerCallback timercb, double seconds);

    void addTimer(TimerPtr timer);

    Timestamp getNextExpiredTime() const;

    void getExpiredTimes(std::vector<TimerPtr>& expired_times);

private:
    struct TimerPtrLess {
    public:
        bool operator()(const TimerPtr& left, const TimerPtr& right) const;
    };

private:
    using TimerMultiSet = std::multiset<TimerPtr, TimerPtrLess>;
    TimerMultiSet timers_;
};

using TimerQueuePtr = std::unique_ptr<TimerQueue>;
}

#endif