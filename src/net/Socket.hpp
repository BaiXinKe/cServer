#ifndef _SOCKET_HPP_
#define _SOCKET_HPP_

#include <memory>
#include <sys/types.h>
#include <unistd.h>

namespace chauncy {
using Handler = int;

enum class SocketType {
    IPv4,
    IPv6
};

class Buffer;

class Socket {
public:
    explicit Socket(SocketType type);

    void setnonblocking();

    void shutdownWR();

    ssize_t read(std::unique_ptr<Buffer>& buffer);
    ssize_t write(std::unique_ptr<Buffer>& buffer);

    Handler fd() const { return sockfd_; }

    ~Socket() { ::close(sockfd_); }

private:
    Handler sockfd_;
};
}

#endif
