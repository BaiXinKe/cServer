#ifndef TCPCONNECTION_HPP__
#define TCPCONNECTION_HPP__

#include "Buffer.hpp"
#include "Channel.hpp"
#include "Handler.hpp"
#include "InetAddr.hpp"
#include "Socket.hpp"

#include "Callbacks.hpp"

#include <any>
#include <atomic>
#include <memory>

namespace Duty {

class EventLoop;
class Channel;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop* loop, const std::string& name, Handler handler, const InetAddr& localaddr, const InetAddr& peeraddr);

    void send(const void* data, size_t len);
    void send(std::string_view message);
    void send(Buffer* buffer);

    void sendInLoop(std::string_view message);
    void sendInLoop(const void* data, size_t len);

    void shutdown();
    void shutdownInLoop();
    void forceClose();
    void forceCloseWithDelay(double seconds);
    void forceCloseInLoop();

    void setTcpNoDelay(bool on);
    void startRead();
    void startReadInLoop();
    void stopRead();
    void stopReadInLoop();

    void connectEstablished();

    void connectDestroyed();

    void setConnectionCallback(ConnectionCallback cb)
    {
        connectioncb_ = cb;
    }

    void setMessageCallback(MessageCallback cb)
    {
        messagecb_ = cb;
    }

    void setWriteCompeleteCallback(WriteCompleteCallback cb)
    {
        writeCompletecb_ = cb;
    }

    void setCloseCallback(CloseCallback cb)
    {
        closecb_ = cb;
    }

    const InetAddr* getLocalAddr() const
    {
        return &localAddr_;
    }

    const InetAddr* getPeerAddr() const
    {
        return &peerAddr_;
    }

    EventLoop* getLoop()
    {
        return loop_;
    }
    std::string name() const
    {
        return name_;
    }

    bool connected() const
    {
        return state_ == State::kConnected;
    }

private:
    void handleRead();
    void handleWrite();
    void handleError();
    void handleClose();

    enum class State {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };

private:
    State state_;

    EventLoop* loop_;
    const std::string name_;
    Handler handler_;

    std::unique_ptr<Socket> socket_;

    const InetAddr localAddr_;
    const InetAddr peerAddr_;

    ConnectionCallback connectioncb_;
    MessageCallback messagecb_;
    WriteCompleteCallback writeCompletecb_;
    HighWaterMarkCallback highWaterMarkcb_;
    CloseCallback closecb_;

    std::unique_ptr<Channel> channel_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;

    size_t highWaterMark_;

    std::any context_;

    std::atomic<bool> reading_;
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

}

#endif