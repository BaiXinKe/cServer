#include "Poller.hpp"
#include "Channel.hpp"
#include <cassert>
#include <spdlog/spdlog.h>
#include <sys/epoll.h>

constexpr int Duty::Poller::EventListInitSize;

Duty::Poller::Poller()
    : epollfd_ { ::epoll_create1(EPOLL_CLOEXEC) }
    , activated_(EventListInitSize)
{
    assert(epollfd_ > 0);
}

void Duty::Poller::poll(ActivateChannls& activateChannels, Timestamp expired)
{
    int64_t timeout = (expired == Timestamp()) ? -1 : getDiffAtNow(expired);

    int numActivated = ::epoll_wait(this->epollfd_, &activated_[0], activated_.size(), timeout);
    if (numActivated == 0)
        return;
    if (numActivated == -1) {
        spdlog::critical("epoll_wait error: " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }

    fillActivateChannels(activateChannels, numActivated);
    if (numActivated == activated_.size()) {
        activated_.resize(numActivated * 2);
    }
}

void Duty::Poller::fillActivateChannels(ActivateChannls& activateChannels, int numActivate)
{
    for (int i = 0; i < numActivate; i++) {
        int activatedFd = activated_[i].data.fd;

        assert(channels_.find(activatedFd) != channels_.end());
        Channel* channel = channels_[activatedFd];
        assert(channel->getState() == Channel::State::REG);

        channel->setRevent(activated_[i].events);
        activateChannels.push_back(channel);
    }
}

void Duty::Poller::update(Channel* channel)
{

    int op {};
    if (channel->getState() == Channel::State::NO_REG) {
        assert(channels_.find(channel->fd()) == channels_.end());

        channels_[channel->fd()] = channel;
        channel->setState(Channel::State::REG);

        op = EPOLL_CTL_ADD;
    } else if (channel->getState() == Channel::State::REG) {

        op = EPOLL_CTL_MOD;

    } else {
        assert(channels_.find(channel->fd()) != channels_.end());
        channels_.erase(channel->fd());
        op = EPOLL_CTL_DEL;
    }

    channelModOperation(channel, op);
}

void Duty::Poller::removeChannel(Channel* channel)
{
    int fd = channel->fd();
    if (channels_.find(fd) == channels_.end()) {
        return;
    }
    channels_.erase(fd);
    if (::epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, NULL) == -1) {
        spdlog::warn("removeChannel epoll_ctl error: " + std::string(strerror(errno)));
    }
}

void Duty::Poller::channelModOperation(Channel* channel, int op)
{
    int fd = channel->fd();
    epoll_event evt;
    evt.data.fd = channel->fd();
    evt.events = channel->getEvent();

    if (::epoll_ctl(epollfd_, op, fd, &evt) != -1)
        return;

    if (op == EPOLL_CTL_DEL) {
        spdlog::warn("epoll operation \"{}\"  not success, because of \"{}\"", op2String(op), std::string(strerror(errno)));
    } else {
        spdlog::error("epoll operation \"{}\"  not success, because of \"{}\"", op2String(op), std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }
}

std::string Duty::Poller::op2String(int op)
{
    switch (op) {
    case EPOLL_CTL_ADD:
        return "EPOLL_CTL_ADD";
    case EPOLL_CTL_MOD:
        return "EPOLL_CTL_MOD";
    case EPOLL_CTL_DEL:
        return "EPOLL_CTL_DEL";
    }
    return "UNKNOWN OPERATOR";
}