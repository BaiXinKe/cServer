#include "Channel.hpp"
#include "EventLoop.hpp"
#include <cassert>

#include <spdlog/spdlog.h>

constexpr int Duty::Channel::empty_event_;

Duty::Channel::Channel(EventLoop* loop, Handler fd)
    : loop_ { loop }
    , handler_ { fd }
    , event_ { empty_event_ }
    , revent_ { empty_event_ }
    , eventHanding_ { false }
    , tied_ { false }
    , writing_ { false }
    , reading_ { false }
{
    assert(loop_ != nullptr);
}

void Duty::Channel::update()
{
    loop_->update(this);
}

void Duty::Channel::tie(const std::weak_ptr<void>& ptr)
{
    tie_ = ptr;
    tied_ = true;
}

void Duty::Channel::remove()
{
    loop_->removeChannel(this);
}

Duty::Channel::~Channel()
{
    this->remove();
}

void Duty::Channel::handleEvent()
{
    eventHanding_ = true;
    std::shared_ptr<TcpConnection> ptr;
    if (tied_) {
        ptr = std::static_pointer_cast<TcpConnection>(tie_.lock());
        if (ptr == nullptr)
            return;
    }

    if ((revent_ & EPOLLHUP) && !(revent_ & EPOLLIN)) {
        if (closecb_)
            closecb_();
    }

    if (revent_ & (EPOLLERR)) {
        if (errorcb_)
            errorcb_();
    }

    if (revent_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
        if (readcb_)
            readcb_();

    if (revent_ & EPOLLOUT)
        if (writecb_)
            writecb_();

    eventHanding_ = false;
}
