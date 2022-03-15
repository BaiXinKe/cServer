#include "HttpServer.hpp"

#include "Buffer.hpp"
#include "HttpContext.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

#include "mime_types.hpp"

#include <spdlog/spdlog.h>

#include <filesystem>

#include <zlib.h>

namespace Duty {

void defaultHttpCallback(const HttpRequest& req, HttpResponse* resp)
{
    resp->setStatusCode(HttpResponse::k401AuthorizationRequired);
    resp->setStatusMessage("Authorization Required");

    char timeBuf[32] {};
    time_t t = time(nullptr);
    ctime_r(&t, timeBuf);

    resp->addHeader("Date", std::string(timeBuf, strlen(timeBuf) - 1));
    resp->addHeader("Server", "cServer/0.1");
    resp->addHeader("WWW-Autenticate", "Basic realm=\"input Your ID and Password\"");
}

HttpServer::HttpServer(EventLoop* loop, const InetAddr& listenAddr, const std::string& name)
    : server_ { loop, listenAddr, name }
    , httpCallback_ { defaultHttpCallback }
{
    server_.setConnectionCallback([this](const TcpConnectionPtr& conn) { this->onConnection(conn); });
    server_.setMessageCallback([this](const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime) {
        this->onMessage(conn, buf, receiveTime);
    });
}

void HttpServer::start()
{
    spdlog::warn("HttpServer [{}] start listenning on {}", server_.name(), server_.ipPort());
    server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected()) {
        conn->setContext(HttpContext());
    } else {
        spdlog::info("Connection: [{}:{}] disconnection", conn->name(), conn->getPeerAddr()->toIpPort());
    }
}

void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
{
    HttpContext* context = std::any_cast<HttpContext>(conn->getMultableContext());
    if (!context)
        return;

    if (!context->parseRequest(buf, receiveTime)) {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }

    if (context->gotAll()) {
        onRequest(conn, context->request());
        context->reset();
    }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
    const std::string& connection = req.getHeader("Connection");
    bool close = connection == "close" || (req.getVersion() == HttpRequest::kHttp10 && connection != "keep-Alive");
    HttpResponse response(close);
    httpCallback_(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);

    conn->send(&buf);

    if (response.closeConnection()) {
        conn->shutdown();
    }
}

}