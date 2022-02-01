#include "Epoll.h"
#include "Channel.h"
#include <cassert>
#include <sys/epoll.h>
#include <system_error>
#include <unistd.h>

namespace cServer::net {

constexpr int Epoll::INIT_EPOLL_EVENTS;

Epoll::Epoll()
    : epollFd_ { epoll_create1(EPOLL_CLOEXEC) }
    , events_(INIT_EPOLL_EVENTS)
{
    if (epollFd_ == -1)
        throw std::system_error { errno, std::system_category(), "epoll_create" };
}

Epoll::~Epoll()
{
    close(epollFd_);
}

void Epoll::wait(std::vector<Channel*>& activateChannels, int timeout)
{
    ssize_t activeNumber = ::epoll_wait(this->epollFd_, &events_[0], events_.size(), timeout);
    if (activeNumber == -1) {
        if (errno == EINTR)
            return;
        throw std::system_error { errno, std::system_category(), "epoll_wait" };
    }
    if (activeNumber == events_.size())
        events_.resize(events_.size() * 2);

    for (int i = 0; i < activeNumber; ++i) {
        int fd = events_[i].data.fd;
        channelMaps_[fd]->setRevent(events_[i].events);
        activateChannels.push_back(channelMaps_[fd]);
    }
}

void Epoll::epoll_add_event(Channel* channel, int event)
{
    channelMaps_[channel->fd()] = channel;
    struct epoll_event ev;
    ev.data.fd = channel->fd();
    ev.events = event;
    if (::epoll_ctl(this->epollFd_, EPOLL_CTL_ADD, channel->fd(), &ev) == -1) {
        throw std::system_error { errno, std::system_category(), "epoll_ctl_add" };
    }
}

void Epoll::epoll_ctl_event(Channel* channel, int event)
{
    assert(channelMaps_[channel->fd()] != nullptr);

    int fd = channel->fd();
    struct epoll_event ev;
    ev.data.fd = channel->fd();
    ev.events = event;
    if (::epoll_ctl(this->epollFd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
        throw std::system_error { errno, std::system_category(), "epoll_ctl_mod" };
    }
}

void Epoll::epoll_ctl_event(Channel* channel, int event)
{
    assert(channelMaps_[channel->fd()] != nullptr);

    int fd = channel->fd();
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = event;
    if (::epoll_ctl(this->epollFd_, EPOLL_CTL_DEL, fd, &ev) == -1) {
        throw std::system_error { errno, std::system_category(), "epoll_ctl_del" };
    }
}

}