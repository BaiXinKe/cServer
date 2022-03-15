#include "ListenSocket.hpp"
#include "EventLoop.hpp"
#include <cassert>
#include <netinet/tcp.h>
#include <spdlog/spdlog.h>

void Duty::defaultServerBusyCallback(int fd)
{
    const char* msg = "Server Current So Busy";
    ::send(fd, msg, strlen(msg), 0);
}

int Duty::ListenSocket::createAndBindSocket(const InetAddr& inetaddr, ProtocolType protocol)
{
    int family = inetaddr.type() == InetType::IPv4 ? AF_INET : AF_INET6;
    int type = protocol == ProtocolType::TCP ? SOCK_STREAM : SOCK_DGRAM;
    int sockfd = ::socket(family, type | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
    assert(sockfd >= 0);

    int op { 1 };
    if (::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op)) == -1) {
        spdlog::critical("setsockopt error {} - {}", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (::bind(sockfd, inetaddr.GetSockaddr(), inetaddr.GetSize()) == -1) {
        spdlog::error("::bind the socket: " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

Duty::ListenSocket::ListenSocket(EventLoop* loop, InetAddr inet_addr, ProtocolType protocol)
    : Socket(createAndBindSocket(inet_addr, protocol))
    , loop_ { loop }
    , channel_ { loop, handler_ }
    , savefd_ { ::open("/dev/null", O_RDONLY) }
    , protocol_ { protocol }
    , busyCallback_ { defaultServerBusyCallback }
{
    assert(handler_ >= 0);
    this->setReuseAddr();
    // this->setNonblocking();
    this->setReusePort();
    this->setTcpNoDelay(true);

    channel_.setReadCallback([this] { this->handleRead(); });
}

void Duty::ListenSocket::handleRead()
{
    loop_->assertInLoopThread();

    sockaddr_storage saddr;
    socklen_t slen = sizeof(saddr);

    int connfd = ::accept4(handler_, reinterpret_cast<sockaddr*>(&saddr), &slen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd == -1) {
        if (errno == EMFILE) {
            ::close(savefd_);
            connfd = ::accept4(handler_, NULL, NULL, 0);
            if (busyCallback_)
                busyCallback_(connfd);
            ::close(connfd);
            ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        } else if (errno == EWOULDBLOCK || errno == EINTR) {
            return;
        } else {
            spdlog::error("accept4 error: " + std::string(strerror(errno)));
        }
    } else {

        InetAddr inet { *reinterpret_cast<sockaddr*>(&saddr), slen };

        if (newConnCallback_)
            newConnCallback_(connfd, inet);
        else {
            ::close(connfd);
        }
    }
}

void Duty::ListenSocket::listen(int linq)
{
    if (::listen(handler_, linq) == -1) {
        spdlog::error("listen error: " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }
    listening_ = true;
    channel_.enableRead();
}

Duty::ListenSocket::~ListenSocket()
{
    if (::close(handler_) == -1) {
        spdlog::warn("::close litensockfd error: " + std::string(strerror(errno)));
    }
    channel_.disableAll();
}

void Duty::ListenSocket::setServerBusyCallback(ServerBusyCallback cb)
{
    busyCallback_ = std::move(cb);
}

void Duty::ListenSocket::setNewConnectionCallback(NewConnectionCallback cb)
{
    newConnCallback_ = std::move(cb);
}