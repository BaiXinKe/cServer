#ifndef POLLER_HPP__
#define POLLER_HPP__

#include <string>
#include <sys/epoll.h>
#include <unordered_map>
#include <vector>

#include "Timestamp.hpp"

namespace Duty {

class Channel;
using ActivateChannls = std::vector<Channel*>;

class Poller {
public:
    static constexpr int EventListInitSize { 32 };

    Poller();

    void poll(ActivateChannls& activateChannels, Timestamp expired);
    void update(Channel* channel);
    void removeChannel(Channel* channel);

private:
    void fillActivateChannels(ActivateChannls& activateChannels, int numActivate);
    void channelModOperation(Channel* channel, int op);

    static std::string op2String(int op);

private:
    using ChannelMaps = std::unordered_map<int, Channel*>;
    using EventList = std::vector<epoll_event>;

    int epollfd_;
    EventList activated_;
    ChannelMaps channels_;
};

}

#endif