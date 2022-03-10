#ifndef TCPSERVER_HPP__
#define TCPSERVER_HPP__

#include "Callbacks.hpp"
#include "ListenSocket.hpp"
#include "TcpConnection.hpp"
#include <atomic>

#include <map>

namespace Duty {

class ListenSocket;
class EventLoop;
class EventLoopThreadPool;

class TcpServer {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    TcpServer(EventLoop* loop, const InetAddr& listenAddr, const std::string& name, ProtocolType protocol = ProtocolType::TCP);
    ~TcpServer();

    const std::string& ipPort() const { return ipPort_; }
    const std::string& name() const { return name_; }
    EventLoop* getLoop() const { return loop_; }

    void setThreadNum(int numThreads);
    void setThreadInitCallback(const ThreadInitCallback& cb)
    {
        threadInitCallback_ = cb;
    }

    std::shared_ptr<EventLoopThreadPool> threadPool()
    {
        return threadPool_;
    }

    void start();

    void setConnectionCallback(const ConnectionCallback& cb)
    {
        connectionCallback_ = cb;
    }

    /// Set message callback.
    /// Not thread safe.
    void setMessageCallback(const MessageCallback& cb)
    {
        messageCallback_ = cb;
    }

    /// Set write complete callback.
    /// Not thread safe.
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    {
        writeCompleteCallback_ = cb;
    }

private:
    void newConnection(int sockfd, const InetAddr& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    EventLoop* loop_; // acceptor loop;
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<ListenSocket> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;

    InetAddr localAddr_;

    std::atomic<bool> start_;

    int nextConnId_;
    ConnectionMap connections_;
};

}

#endif