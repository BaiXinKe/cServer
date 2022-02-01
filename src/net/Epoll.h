#ifndef _CSERVER_EPOLL_H_
#define _CSERVER_EPOLL_H_

#include <sys/epoll.h>
#include <unordered_map>

namespace cServer::net {
class Channel;

class Epoll {
private:
    int epollFd_;
    std::vector<epoll_event> events_;
    std::unordered_map<int, Channel*> channelMaps_;

    static constexpr int INIT_EPOLL_EVENTS { 128 };

public:
    Epoll();
    ~Epoll();

    void wait(std::vector<Channel*>& activateChannels, int timeout);

    void epoll_add_event(Channel* channel, int event);
    void epoll_ctl_event(Channel* channel, int event);
    void epoll_del_event(Channel* channel, int event);
};
}
#endif