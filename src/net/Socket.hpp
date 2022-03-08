#ifndef SOCKET_HPP__
#define SOCKET_HPP__

#include "Handler.hpp"

namespace Duty {

class Socket {
public:
    Socket(int domain, int socktype, int protocol);
    explicit Socket(int handler);
    void setTcpNoDelay(bool on);

    void setLinger(bool on, int linger);

    void setReuseAddr();
    void setReusePort();
    void setNonblocking();

    void setKeepAlive(bool on);

    void shutdownWR();

protected:
    Handler handler_;
};

}

#endif