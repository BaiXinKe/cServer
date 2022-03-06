#ifndef TIMER_HPP__
#define TIMER_HPP__

#include <functional>
#include <memory>

#include "Callbacks.hpp"
#include "TimerId.hpp"
#include "Timestamp.hpp"

namespace Duty {

class Timer {
public:
    Timer(TimerCallback timercb, Timestamp expire_time, std::chrono::milliseconds interval = {});
    void run();

    void restart();

    bool repeat() const { return repeat_; }
    std::chrono::milliseconds interval() const { return interval_; }
    Timestamp expTime() const { return expire_time_; }
    bool expired() const;

    TimerId id() const { return id_; }

    friend bool operator<(const Timer& left, const Timer& right);

private:
    TimerId id_;
    TimerCallback timercb_;

    Timestamp expire_time_;
    std::chrono::milliseconds interval_;
    bool repeat_;
};

using TimerPtr = std::unique_ptr<Timer>;

inline bool
operator<(const Timer& left, const Timer& right)
{
    return left.expire_time_ < right.expire_time_;
}

inline bool
operator==(const Timer& left, const Timer& right)
{
    return left.id() == right.id();
}

inline bool
operator==(const TimerPtr& left, const TimerPtr& right)
{
    return (*left) == (*right);
}

}

#endif