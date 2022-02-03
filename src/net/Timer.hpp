#pragma once

#include <chrono>
#include <functional>
#include <memory>

#include "TimerId.hpp"

using Timestamp = std::chrono::system_clock::time_point;
using TimerCallback = std::function<void()>;

class Timer {
public:
    Timer(TimerCallback cb, Timestamp expiration, std::chrono::milliseconds interval);
    Timer(TimerCallback cb, Timestamp expiration, std::chrono::seconds interval);

    ~Timer() = default;

    bool repeat() const { return repeat_; }
    std::chrono::milliseconds interval() const { return interval_; }
    std::chrono::seconds intervalAsSeconds() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(interval_);
    }
    Timestamp expiration() const { return expiration_; }
    TimerId id() const { return id_; }

    void reset();

    void run();

    inline bool operator<(const Timer& other);

private:
    TimerId id_;
    TimerCallback timercb_;
    Timestamp expiration_;
    std::chrono::milliseconds interval_;
    bool repeat_;
};

inline bool
Timer::operator<(const Timer& other)
{
    bool flag = this->expiration_ < other.expiration_;
    return flag ? true : (this->expiration_ == other.expiration_ ? (this->id_ < other.id_) : false);
}

using TimerPtr = std::unique_ptr<Timer>;

inline bool
operator<(const TimerPtr& left, const TimerPtr& right)
{
    bool flag = left->expiration() < right->expiration();
    return flag ? true : (left->expiration() == right->expiration() ? (left->id() < right->id()) : false);
}