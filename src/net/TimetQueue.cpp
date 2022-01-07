#include "TimerQueue.h"

namespace cServer::net {

void TimerQueue::push(TimerPtr timer)
{
    timers_.insert(std::move(timer));
}

template <typename Func, typename... Args>
void TimerQueue::runAt(base::Timestamp ts, Func&& func, Args... args)
{
    TimerCallback cb = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
    TimerPtr timer = std::make_shared<Timer>(std::move(cb), ts);
    timers_.insert(std::move(timer));
}

template <typename Func, typename... Args>
void TimerQueue::runAfter(double seconds, Func&& func, Args... args)
{
    base::Timestamp expired = base::Timestamp::afterSeconds(seconds);
    TimerCallback cb = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
    TimerPtr timer = std::make_shared<Timer>(std::move(cb), expired);
    timers_.insert(std::move(timer));
}

template <typename Func, typename... Args>
void TimerQueue::runAtInterval(base::Timestamp ts, double interval, Func&& func, Args... args)
{
    TimerCallback cb = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
    TimerPtr timer = std::make_shared<Timer>(std::move(cb), ts, interval);
    timers_.insert(std::move(timer));
}

template <typename Func, typename... Args>
void TimerQueue::runAfterInterval(double seconds, double interval, Func&& func, Args... args)
{
    base::Timestamp expired = base::Timestamp::afterSeconds(seconds);
    TimerCallback cb = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
    TimerPtr timer = std::make_shared<Timer>(std::move(cb), expired, interval);
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