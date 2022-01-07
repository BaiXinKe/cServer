#include "TimerQueue.h"

namespace cServer::net {

void TimerQueue::push(TimerPtr timer)
{
    timers_.insert(std::move(timer));
}

std::vector<TimerPtr> TimerQueue::get_expired_timers()
{
    std::vector<TimerPtr> res;
    TimerPtr now { Timer::now() };
    auto ptr = timers_.lower_bound(now);
    if (ptr != timers_.end()) {
        res.assign(timers_.begin(), ptr);
        timers_.erase(timers_.begin(), ptr);
    }
    return res;
}

}