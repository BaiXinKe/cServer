#include "EPoller.hpp"
#include "Channel.hpp"
#include "Timestamp.hpp"

#include <spdlog/spdlog.h>

namespace Duty {

namespace {

    std::string opToString(int op)
    {
        switch (op) {
        case EPOLL_CTL_ADD:
            return "EPOLL_CTL_ADD";
        case EPOLL_CTL_DEL:
            return "EPOLL_CTL_DEL";
        case EPOLL_CTL_MOD:
            return "EPOLL_CTL_MOD";
        }

        return "Unknown";
    }

}

Epoller::Epoller(size_t activeVecLength)
    : epollfd_ { ::epoll_create1(EPOLL_CLOEXEC) }
    , activatedFds_(activeVecLength)
{
    assert(epollfd_ > 0);
    assert(activatedFds_.size() == activeVecLength);
}

void Epoller::update(Channel* channel)
{
    int op {};
    if (channels_.find(channel->fd()) == channels_.end()) {
        op = EPOLL_CTL_ADD;
        channels_[channel->fd()] = channel;
    } else {
        op = EPOLL_CTL_MOD;
    }

    epoll_event ev;
    ev.data.ptr = channel;
    ev.events = channel->getEvent();

    if (::epoll_ctl(epollfd_, op, channel->fd(), &ev) == -1) {
        spdlog::critical("epoll_ctl({}) error {}", opToString(op), strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void Epoller::wait(std::vector<Channel*>* activatedChannels, Timestamp expiredTime)
{
    int expire { -1 };
    if (expiredTime != Timestamp()) {
        expire = getDiffAtNow(expiredTime);
    }
    ssize_t activedNum = ::epoll_wait(epollfd_, &activatedFds_[0], activatedFds_.size(), expire);
    if (activedNum == 0)
        return;
    if (activedNum == -1 && errno != EINTR) {
        spdlog::critical("::epoll_wait error: {}", strerror(errno));
        exit(EXIT_FAILURE);
    }

    fillActivatedChannels(activatedChannels, activedNum);

    if (activatedFds_.size() == static_cast<size_t>(activedNum)) {
        activatedFds_.resize(2 * activatedFds_.size());
    }
}

void Epoller::fillActivatedChannels(std::vector<Channel*>* channelsNeedFill, int numToFill)
{
    for (int i = 0; i < numToFill; i++) {
        Channel* channel = reinterpret_cast<Channel*>(activatedFds_[i].data.ptr);
        channel->setRevent(activatedFds_[i].events);

        assert(channel != nullptr);
        assert(channels_.find(channel->fd()) != channels_.end());

        channelsNeedFill->push_back(channel);
    }
}

void Epoller::removeChannel(Channel* channel)
{
    if (channels_.find(channel->fd()) == channels_.end())
        return;

    assert(channels_.find(channel->fd()) != channels_.end());
    int fd = channel->fd();
    channels_.erase(fd);

    if (::epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, nullptr) == -1) {
        spdlog::warn("epoll_ctl (del) error: {}", strerror(errno));
    }
}

Epoller::~Epoller()
{
    if (::close(epollfd_) == -1) {
        spdlog::warn("::close epoll error: {}", strerror(errno));
    }
}

}