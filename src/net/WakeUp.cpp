#include "Wakeup.hpp"
#include <cassert>
#include <spdlog/spdlog.h>
#include <sys/eventfd.h>
#include <unistd.h>

Duty::WakeUp::WakeUp(EventLoop* loop)
    : eventfd_ { eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK) }
    , channel_(loop, eventfd_)
{
    assert(eventfd_ > 0);
    channel_.setReadCallback([this] {
        this->handleRead();
    });
    channel_.enableRead();
}

void Duty::WakeUp::handleRead()
{
    uint64_t val;
    if (::read(eventfd_, &val, sizeof(uint64_t)) == -1 && errno != EAGAIN) {
        spdlog::error("read val from eventfd error " + std::string(strerror(errno)));
    }
}

void Duty::WakeUp::handleWrite()
{
    uint64_t val { 1 };
    if (::write(eventfd_, &val, sizeof(uint64_t)) != sizeof(uint64_t)) {
        spdlog::error("write val to eventfd error " + std::string(strerror(errno)));
    }
}

Duty::WakeUp::~WakeUp()
{
    if (::close(eventfd_) == -1) {
        spdlog::warn("Close eventfd error" + std::string(strerror(errno)));
    }
}