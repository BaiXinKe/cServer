#ifndef TCPCONNECTION_HPP__
#define TCPCONNECTION_HPP__

#include "Callbacks.hpp"
#include "Channel.hpp"
#include "Handler.hpp"
#include "InetAddr.hpp"

#include <any>
#include <memory>
#include <string>
#include <string_view>

namespace Duty {

class InetAddr;
class Buffer;
class Socket;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    enum class State {
        Connecting,
        Connected,
        Disconnecting,
        Disconnected
    };

    TcpConnection(EventLoop* loop, std::string_view name, Handler handler,
        const InetAddr& localAddr, const InetAddr& peerAddr);

    void send(const void* data, size_t len);
    void send(std::string_view data);
    void send(Buffer* buff);

    void sendInLoop(std::string_view data);
    void sendInLoop(const void* data, size_t len);

    void connectEstablished();

    void connectDestroyed();

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

    void setContext(const std::any& context)
    {
        context_ = context;
    }

    const std::any& getContext() const
    {
        return context_;
    }

    std::any* getMultableContext()
    {
        return &context_;
    }

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
        return state_ == State::Connected;
    }

    ~TcpConnection();

private:
    void handleRead();
    void handleWrite();
    void handleError();
    void handleClose();

private:
    State state_;
    EventLoop* loop_;

    const std::string name_;

    Handler handler_;

    Channel channel_;

    std::unique_ptr<Socket> socket_;

    const InetAddr localAddr_;
    const InetAddr peerAddr_;

    ConnectionCallback connectioncb_;
    MessageCallback messagecb_;
    WriteCompleteCallback writeCompletecb_;
    CloseCallback closecb_;

    size_t highWaterMark_;
    HighWaterMarkCallback highWaterMarkcb_;

    std::unique_ptr<Buffer> inputBuffer_;
    std::unique_ptr<Buffer> outputBuffer_;

    std::any context_;
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
}

#endif