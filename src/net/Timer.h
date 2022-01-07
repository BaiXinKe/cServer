#ifndef _CSERVER_TIMER_H_
#define _CSERVER_TIMER_H_

#include "../base/Timestamp.h"
#include "../base/noncopyable.hpp"
#include <atomic>
#include <functional>
#include <memory>

namespace cServer::net {

using TimerCallback = std::function<void()>;

class Timer;
using TimerPtr = std::shared_ptr<Timer>;

class Timer : base::noncopyable {
private:
    static std::atomic<uint64_t> global_id_;
    base::Timestamp ts_;
    uint64_t id_;
    TimerCallback timercb_;
    bool repeat_;
    double interval_;

    Timer(base::Timestamp expireation);

public:
    friend inline bool operator<(const TimerPtr&, const TimerPtr&);

    static TimerPtr now();

    Timer(TimerCallback cb, base::Timestamp expireation, double interval = 0);

    bool isRepeat() const noexcept
    {
        return repeat_;
    }

    double interval() const noexcept
    {
        return interval_;
    }

    bool isExpired() const noexcept;

    void run();

    uint64_t id() const
    {
        return id_;
    }

    void cancel();
};

inline bool
operator<(const TimerPtr& left, const TimerPtr& right)
{
    if (left->ts_ == right->ts_)
        return left->id_ < right->id_;
    return left->ts_ < right->ts_;
}

}

#endif