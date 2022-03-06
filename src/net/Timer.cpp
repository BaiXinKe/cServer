#include "Timer.hpp"

Duty::Timer::Timer(TimerCallback timercb, Timestamp expire_time, std::chrono::milliseconds interval)
    : id_ {}
    , timercb_ { timercb }
    , expire_time_ { expire_time }
    , interval_ { interval }
    , repeat_ { interval != std::chrono::milliseconds {} }
{
}

void Duty::Timer::run()
{
    if (timercb_) {
        timercb_();
    }
}

void Duty::Timer::restart()
{
    if (repeat_) {
        expire_time_ += interval_;
    } else {
        expire_time_ = Timestamp {};
    }
}

bool Duty::Timer::expired() const
{
    return expire_time_ >= now();
}
