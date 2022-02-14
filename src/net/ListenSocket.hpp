#ifndef LISTENSOCKET_HPP__
#define LISTENSOCKET_HPP__

#include "Socket.hpp"
#include <sys/socket.h>

namespace chauncy {

class InetAddr;

class ListenSocket {
public:
    explicit ListenSocket(SocketType type);
    void bind(const InetAddr& addr);
    void listen(int max_conn = SOMAXCONN);

    Handler accept(InetAddr& addr);

private:
    Socket socket_;
    Handler dummyfd_;
};

}
#endif