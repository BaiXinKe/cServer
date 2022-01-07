#ifndef _CSERVER_TIMER_QUEUE_
#define _CSERVER_TIMER_QUEUE_

#include "../base/noncopyable.hpp"
#include "../net/Timer.h"
#include <set>

namespace cServer::net {

class TimerQueue : base::noncopyable {
public:
    TimerQueue() = default;
    ~TimerQueue() = default;

    void push(TimerPtr timer);

    template <typename Func, typename... Args>
    void runAt(base::Timestamp ts, Func&& func, Args...);

    template <typename Func, typename... Args>
    void runAfter(double seconds, Func&& func, Args...);

    template <typename Func, typename... Args>
    void runAtInterval(base::Timestamp ts, double interval, Func&& func, Args... args);

    template <typename Func, typename... Args>
    void runAfterInterval(double seconds, double interval, Func&& func, Args... args);

    std::vector<TimerPtr> get_expired_timers();

private:
    std::set<TimerPtr> timers_;
};

}

#endif