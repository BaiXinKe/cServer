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
    std::vector<TimerPtr> get_expired_timers();

private:
    std::set<TimerPtr> timers_;
};

}

#endif