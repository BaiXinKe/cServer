#ifndef HTTPSERVER_HPP__
#define HTTPSERVER_HPP__

#include "TcpServer.hpp"
#include <boost/noncopyable.hpp>

namespace Duty {

class HttpRequest;
class HttpResponse;

class HttpServer : boost::noncopyable {
public:
    using HttpCallback = std::function<void(const HttpRequest&, HttpResponse*)>;

    HttpServer(EventLoop* loop, const InetAddr& listenAddr, const std::string& name);

    EventLoop* getLoop() const { return server_.getLoop(); }

    void setHttpCallback(const HttpCallback& cb)
    {
        httpCallback_ = cb;
    }

    void setThreadNum(int numThreads = 0)
    {
        server_.setThreadNum(numThreads);
    }

    void start();

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime);
    void onRequest(const TcpConnectionPtr&, const HttpRequest&);

private:
    TcpServer server_;
    HttpCallback httpCallback_;
};

}

#endif