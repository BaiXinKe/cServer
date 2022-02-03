#include "TimerQueue.hpp"
#include "EventLoop.h"

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_ { loop }
    , timer_list_ {}
{
}

void TimerQueue::addTimer(TimerCallback cb, Timestamp timestamp, std::chrono::seconds second)
{
    addTimer(cb, timestamp, std::chrono::duration_cast<std::chrono::milliseconds>(second));
}

void TimerQueue::addTimer(TimerCallback cb, Timestamp timestamp, std::chrono::milliseconds interval)
{
    Entry entry {};
    entry.first = timestamp;
    entry.second = std::make_unique<Timer>(cb, timestamp, interval);
    timer_list_.insert(std::move(entry));
}

Timestamp TimerQueue::getNextExpirationTimestamp() const
{
    return timer_list_.cbegin()->first;
}

void TimerQueue::cancel(TimerId id)
{
    cancel_timers_.insert(id);
}

TimerQueue::ActivateTimers TimerQueue::getExpireation()
{
    using namespace std::chrono;

    // Complete error, because the entry in set have const characteristic can't to move.
    // loop_->assertInLoopThread();
    // ActivateTimers timers_;
    // Timestamp now = system_clock::now();
    // Entry curr { std::make_pair(now, std::make_unique<Timer>(TimerCallback(), now, milliseconds(0))) };

    // auto where = timer_list_.lower_bound(curr);

    // timer_list_.erase(begin(timer_list_), where);

    // for (auto& t : entrys) {
    //     if (cancel_timers_.find(t.second->id()) != cancel_timers_.end())
    //         continue;
    //     timers_.emplace_back(std::move(t.second));
    // }

    // return timers_;
}

void TimerQueue::reset(ActivateTimers timers)
{
    loop_->assertInLoopThread();
    for (auto& timer : timers) {
        if (timer->repeat()) {
            timer->reset();

            Entry entry;
            entry.first = timer->expiration();
            entry.second = std::move(timer);

            timer_list_.insert(std::move(entry));
        }
    }
}