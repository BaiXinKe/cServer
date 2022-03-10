#ifndef EPOLLER_HPP__
#define EPOLLER_HPP__
#include <unordered_map>
#include <vector>

#include <sys/epoll.h>

#include "Timestamp.hpp"

namespace Duty {
class Channel;

class Epoller {
public:
    static constexpr size_t DEFAULT_INIT_EPOLL_EVENT { 32 };

    explicit Epoller(size_t activeVecLength = DEFAULT_INIT_EPOLL_EVENT);

    void update(Channel* channel);

    void wait(std::vector<Channel*>* activatedChannels, Timestamp expiredTime);

    void removeChannel(Channel* channel);

    ~Epoller();

private:
    void fillActivatedChannels(std::vector<Channel*>* channelsNeedFille, int num2fill);

private:
    using EpollActivated = std::vector<epoll_event>;
    using ChannelMaps = std::unordered_map<int, Channel*>;

    int epollfd_;

    EpollActivated activatedFds_;
    ChannelMaps channels_;
};
}

#endif