#include "Timer.h"

namespace cServer::net {

std::atomic<uint64_t> Timer::global_id_ { 0 };
TimerPtr Timer::now()
{
    base::Timestamp now = base::Timestamp::now();
    TimerPtr pnow = std::make_shared<Timer>(now);
    return pnow;
}

Timer::Timer(TimerCallback cb, base::Timestamp expireation, double interval)
    : id_ { global_id_.fetch_add(1) }
    , ts_ { expireation }
    , timercb_ { std::move(cb) }
    , repeat_ { interval != 0 }
    , interval_ { interval_ }
{
}

Timer::Timer(base::Timestamp expireation)
    : ts_ { expireation }
{
}

bool Timer::isExpired() const noexcept
{
    base::Timestamp now = base::Timestamp::now();
    return this->ts_ <= now;
}

void Timer::run()
{
    if (this->timercb_) {
        this->timercb_();
    }
}

void Timer::cancel()
{
    this->timercb_ = {};
}

}