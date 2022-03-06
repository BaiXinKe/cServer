#include "Socket.hpp"
#include <cassert>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <spdlog/spdlog.h>
#include <sys/socket.h>

Duty::Socket::Socket(int domain, int socktype, int protocol)
    : handler_ { ::socket(domain, socktype, protocol) }
{
    assert(handler_ > 0);
}

Duty::Socket::Socket(int handler)
    : handler_ { handler }
{
    this->setNonblocking();
    this->setReuseAddr();
    this->setReusePort();
    this->setTcpNoDelay(true);
}

void Duty::Socket::setReuseAddr()
{
    int opt { 1 };
    if (::setsockopt(handler_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1) {
        spdlog::error("setsockopt :" + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }
}

void Duty::Socket::setNonblocking()
{
    int flag = ::fcntl(handler_, F_GETFL);
    flag |= O_NONBLOCK;
    if (::fcntl(handler_, F_SETFL) == -1) {
        spdlog::error("::fcntl set nonblocking: " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }
}

void Duty::Socket::setTcpNoDelay(bool on)
{
    int opt { on ? 1 : 0 };
    if (::setsockopt(handler_, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) == -1) {
        spdlog::error("setsocketopt (NO_DELAY) : " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }
}

void Duty::Socket::setReusePort()
{
    int opt { 1 };
    if (::setsockopt(handler_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
        spdlog::error("setsockopt (SO_REUSEPORT): " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }
}

void Duty::Socket::setLinger(bool on, int linger)
{
    struct linger lin;
    lin.l_onoff = on ? 1 : 0;
    lin.l_linger = linger;

    if (::setsockopt(handler_, SOL_SOCKET, SO_LINGER, &lin, sizeof(lin)) == -1) {
        spdlog::error("setsockopt (linger): " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }
}

void Duty::Socket::setKeepAlive(bool on)
{
    int opt { on ? 1 : 0 };
    if (::setsockopt(handler_, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) == -1) {
        spdlog::error("setsocketopt (keep-alive): " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }
}

void Duty::Socket::shutdownWR()
{
    if (::shutdown(handler_, SHUT_WR) == -1) {
        spdlog::error("::shutdown : " + std::string(strerror(errno)));
    }
}