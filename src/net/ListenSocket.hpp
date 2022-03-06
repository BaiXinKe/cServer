#ifndef LISTEN_SOCKET_HPP__
#define LISTEN_SOCKET_HPP__
#include "Callbacks.hpp"
#include "Channel.hpp"
#include "Handler.hpp"
#include "InetAddr.hpp"
#include "Socket.hpp"

namespace Duty {
enum class ProtocolType {
    TCP,
    UDP
};

using ServerBusyCallback = std::function<void(int)>;

class ListenSocket : public Socket {
public:
    explicit ListenSocket(EventLoop* loop, InetAddr inetaddr, ProtocolType protocol);

    void setServerBusyCallback(ServerBusyCallback cb);
    void setNewConnectionCallback(NewConnectionCallback cb);

    void listen(int linq = SOMAXCONN);

    ~ListenSocket();

private:
    void handleRead();

private:
    static int createAndBindSocket(const InetAddr& inetaddr, ProtocolType protocol);

private:
    EventLoop* loop_;
    Channel channel_;

    NewConnectionCallback newConnCallback_;

    int savefd_;

    bool listening_;
    InetAddr addr_;
    ProtocolType protocol_;
    ServerBusyCallback busyCallback_;
};

extern void defaultServerBusyCallback(int fd);
}

#endif