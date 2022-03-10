#ifndef EVENTLOOP_HPP__
#define EVENTLOOP_HPP__

#include "EPoller.hpp"
#include "ThreadSafeQueue.hpp"
#include "TimerQueue.hpp"
#include "WakeUp.hpp"

#include <atomic>
#include <memory>

namespace Duty {

class Channel;

class EventLoop {
public:
    using ActivateChannls = std::vector<Channel*>;
    using ExpiredTimers = std::vector<TimerPtr>;

    using PendingTask = std::function<void()>;

public:
    EventLoop();

    void loop();

    void update(Channel* channel);
    void removeChannel(Channel* channel);

    void runAfter(TimerCallback timercb, std::chrono::milliseconds millseconds);
    void runAfter(TimerCallback timercb, double seconds);

    void runUntil(TimerCallback timercb, Timestamp expire_time);
    void runUntil(TimerCallback timercb, time_t expire_time);

    void runEvery(TimerCallback timercb, std::chrono::milliseconds millseconds);
    void runEvery(TimerCallback timercb, double seconds);

    void stop()
    {
        stop_ = true;
        runInLoop([] {});
    }

    ~EventLoop();
    void assertInLoopThread() const;
    bool isInLoopThread() const;

    void runInLoop(PendingTask task);

private:
    void execExpiredTimesTask();

    void doPendingTasks();

private:
    std::atomic<bool> stop_;
    std::unique_ptr<Epoller> poll_;
    ActivateChannls activedChannels_;

    TimerQueuePtr timerTaskQueue_;
    ExpiredTimers expired_timers_;

    std::atomic<bool> runningPending_;
    ThreadSafeQueuePtr<PendingTask> pendingTask_;

    WakeUpPtr wake_;
};

}

#endif
