#include "ListenSocket.hpp"
#include "Error.hpp"
#include "InetAddr.hpp"
#include "Logger.hpp"
#include "Socket.hpp"
#include <sys/eventfd.h>
#include <sys/socket.h>

using namespace chauncy;

ListenSocket::ListenSocket(SocketType type)
    : socket_ { type }
    , dummyfd_ { eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK) }
{
    // socket_.setnonblocking();
}

void ListenSocket::bind(const InetAddr& addr)
{
    if (::bind(socket_.fd(), addr.getaddr(), addr.addrSize()) == -1) {
        std::string err = errMsg("bind");
        CRITICAL(err);
    }
}

void ListenSocket::listen(int max_conn)
{
    if (::listen(socket_.fd(), max_conn) == -1) {
        std::string err = errMsg("listen");
        CRITICAL(err);
    }
}

chauncy::Handler ListenSocket::accept(InetAddr& addr)
{
    socklen_t len = addr.addrSize();
    int connfd = ::accept4(socket_.fd(), addr.getaddr(), &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd == -1) {
        if (errno == EMFILE || errno == ENFILE) {
            ::close(dummyfd_);
            dummyfd_ = ::accept(socket_.fd(), nullptr, nullptr);
            ::close(dummyfd_);
            dummyfd_ = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
            WARN("the number of connect had reached the max open file descriptor number\n");
        } else if (errno == EWOULDBLOCK || errno == EAGAIN) {
            // no action for this
        } else {
            std::string err = errMsg("accept");
            CRITICAL(err);
        }
    }

    return connfd;
}
