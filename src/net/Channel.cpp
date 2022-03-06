#include "Channel.hpp"
#include "EventLoop.hpp"
#include <cassert>

constexpr int Duty::Channel::empty_event_;

Duty::Channel::Channel(EventLoop* loop, Handler fd)
    : loop_ { loop }
    , handler_ { fd }
    , event_ { empty_event_ }
    , revent_ { empty_event_ }
    , state_ { State::NO_REG }
    , writing_ { false }
    , reading_ { false }
{
    assert(loop_ != nullptr);
}

void Duty::Channel::update()
{
    loop_->update(this);
}

void Duty::Channel::handleEvent()
{
    if (revent_ & IN) {
        if (revent_ & EPOLLHUP) {
            if (closecb_) {
                closecb_();
            }
        } else if (readcb_) {
            readcb_();
        }
    }
    if (revent_ & OUT) {

        if (writecb_)
            writecb_();
    }
    if (revent_ & ERR) {
        if (errorcb_)
            errorcb_();
    }
}

void Duty::Channel::remove()
{
    loop_->removeChannel(this);
}

Duty::Channel::~Channel()
{
    this->remove();
}

void Duty::Channel::tie(const std::weak_ptr<TcpConnection>& ptr)
{
    tie_ = ptr;
}