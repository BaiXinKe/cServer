#include "Socket.hpp"
#include "Buffer.hpp"
#include "Logger.hpp"
#include <cassert>
#include <sys/socket.h>

using namespace chauncy;

static void buildErrMsg(const char* functionName, char* buf, size_t bufSize, int errnum)
{
    int ret = snprintf(buf, bufSize, "%s: ", functionName);
    strerror_r(errnum, buf + ret, bufSize - ret);
};

Socket::Socket(SocketType type)
    : sockfd_ { socket(type == SocketType::IPv4 ? AF_INET : AF_INET6, SOCK_STREAM, 0) }
{
    assert(sockfd_ >= 0);
    int val = 1;
    if (setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1) {
        char errMsg[256] {};
        buildErrMsg("setsocketopt", errMsg, sizeof(errMsg), errno);
        CRITICAL(errMsg);
    }
}

void Socket::setnonblocking()
{
    int ret = ::fcntl(sockfd_, F_GETFL);
    if (ret == -1) {
        char errMsg[256] {};
        buildErrMsg("fcntl", errMsg, sizeof(errMsg), errno);
        CRITICAL(errMsg);
    }
    ret |= O_NONBLOCK;
    if (::fcntl(sockfd_, F_SETFL, ret) == -1) {
        char errMsg[256] {};
        buildErrMsg("fcntl-2", errMsg, sizeof(errMsg), errno);
        CRITICAL(errMsg);
    }
}

void Socket::shutdownWR()
{
    if (::shutdown(sockfd_, SHUT_WR) == -1) {
        char errMsg[256] {};
        buildErrMsg("shutdown", errMsg, sizeof(errMsg), errno);
        ERROR(errMsg);
    }
}

ssize_t Socket::read(std::unique_ptr<Buffer>& buffer)
{
    static char stackBuffer[65536] {};
    ssize_t readSize {};

    if ((readSize = ::recv(sockfd_, &stackBuffer, sizeof(stackBuffer), 0)) > 0) {
        buffer->append(stackBuffer, readSize);
    }

    if (readSize < 0 && errno != (EWOULDBLOCK || EAGAIN)) {
        char errMsg[256] {};
        buildErrMsg("read", errMsg, sizeof(errMsg), errno);
        ERROR(errMsg);
    }
    return readSize;
}

ssize_t Socket::write(std::unique_ptr<Buffer>& buffer)
{
    ssize_t writeSize {};
    if ((writeSize = ::send(sockfd_, buffer->peek(), buffer->readableBytes(), 0)) > 0) {
        buffer->retrieve(writeSize);
    }

    if (writeSize < 0 && errno != (EWOULDBLOCK || EAGAIN)) {
        char errMsg[256] {};
        buildErrMsg("write", errMsg, sizeof(errMsg), errno);
        ERROR(errMsg);
    }

    return writeSize;
}