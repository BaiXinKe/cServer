#include "TcpServer.hpp"
#include "EventLoop.hpp"
#include "EventLoopThreadPool.hpp"
#include "ListenSocket.hpp"
#include "sig_pipe.hpp"

#include <spdlog/spdlog.h>

#include <stdio.h>

namespace Duty {

TcpServer::TcpServer(EventLoop* loop, const InetAddr& listenAddr, const std::string& name, ProtocolType protocol)
    : loop_ { loop }
    , ipPort_ {
        listenAddr.toIpPort()
    }
    , name_ { name }
    , acceptor_ { std::make_unique<ListenSocket>(loop_, listenAddr, protocol) }
    , threadPool_ { nullptr }
    , connectionCallback_ { defaultConnectionCallback }
    , messageCallback_ { defaultMessageCallback }
    , localAddr_ { listenAddr }
    , start_ { false }
    , nextConnId_ { 1 }
{
    (void)ignore;
    acceptor_->setNewConnectionCallback(
        [this](int sockfd, const InetAddr& peerAddr) {
            this->newConnection(sockfd, peerAddr);
        });
}

TcpServer::~TcpServer()
{
    loop_->assertInLoopThread();
    spdlog::info("TcpServer::~TcpServer {} destructing", name_);

    for (auto& item : connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop([conn] {
            conn->connectDestroyed();
        });
    }
}

void TcpServer::setThreadNum(int numThreads)
{
    assert(0 <= numThreads);
    threadPool_ = std::make_shared<EventLoopThreadPool>(numThreads, threadInitCallback_);
}

void TcpServer::start()
{
    if (!start_) {
        start_ = true;
        threadPool_->start();
        loop_->runInLoop([this] {
            this->acceptor_->listen();
        });
    }
}

void TcpServer::newConnection(int sockfd, const InetAddr& peerAddr)
{
    loop_->assertInLoopThread();
    EventLoop* ioLoop = threadPool_->getNextEventLoop();
    char buf[64];
    snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    spdlog::info("TcpServer::newConnection [{}] new connection [{}] from {}", name_, connName, peerAddr.toIpPort());

    InetAddr localAddr(this->localAddr_);

    TcpConnectionPtr conn { std::make_shared<TcpConnection>(ioLoop, connName, sockfd, localAddr, peerAddr) };
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setWriteCompeleteCallback(writeCompleteCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback([this](const TcpConnectionPtr& conn) { this->removeConnection(conn); });

    assert(conn != nullptr);

    ioLoop->runInLoop([conn] { conn->connectEstablished(); });
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->runInLoop([this, &conn] { this->removeConnectionInLoop(conn); });
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    spdlog::info("TcpServer::removeConnectionInLoop [{}] connection ", conn->name());

    size_t n = connections_.erase(conn->name());
    (void)n;
    assert(n == 1);
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->runInLoop([&conn] { conn->connectDestroyed(); });
}

}