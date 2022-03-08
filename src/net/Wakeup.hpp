#ifndef WAKEUP_HPP__
#define WAKEUP_HPP__
#include "Channel.hpp"
#include <memory>

namespace Duty {

class EventLoop;

class WakeUp {
public:
    WakeUp(EventLoop* loop);

    ~WakeUp();

    void handleRead();
    void handleWrite();

private:
    int eventfd_;

    Channel channel_;
};

using WakeUpPtr = std::unique_ptr<WakeUp>;

}

#endif