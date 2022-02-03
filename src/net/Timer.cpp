#include "Timer.hpp"
#include <chrono>

Timer::Timer(TimerCallback cb, Timestamp expiration, std::chrono::milliseconds interval)
    : id_ {}
    , timercb_ { cb }
    , expiration_ { expiration }
    , interval_ { interval }
    , repeat_ { interval != std::chrono::milliseconds() }
{
}

Timer::Timer(TimerCallback cb, Timestamp expiration, std::chrono::seconds interval)
    : Timer { cb, expiration, std::chrono::duration_cast<std::chrono::milliseconds>(interval) }
{
}

void Timer::reset()
{
    if (repeat_) {
        expiration_ += std::chrono::milliseconds(interval_);
    } else {
        expiration_ = Timestamp();
    }
}

void Timer::run()
{
    if (timercb_) {
        timercb_();
    }
}
