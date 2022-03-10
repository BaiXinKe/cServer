#include "HttpServer.hpp"

#include "Buffer.hpp"
#include "HttpContext.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

#include <spdlog/spdlog.h>

#include <filesystem>

namespace Duty {

void defaultHttpCallback(const HttpRequest& req, HttpResponse* resp)
{
    namespace fs = std::filesystem;

    std::string filename = req.path();
    if (filename == "/") {
        filename = "./index.html";
    } else if (filename[0] == '/') {
        filename = "." + filename;
    }

    fs::path path { filename };
    fs::directory_entry entry { path };

    if (!entry.exists()) {
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setStatusMessage("Not Found\r\n\r\n");
        resp->setCloseConnection(true);
        return;
    }

    std::string body;
    body.resize(entry.file_size());
    int fd = ::open(filename.c_str(), O_RDONLY);

    auto needRead { entry.file_size() };
    ssize_t readed = ::read(fd, &body[0], body.size());

    if (static_cast<uintmax_t>(readed) < needRead)
        spdlog::warn("partial read");

    resp->setStatusCode(HttpResponse::k200Ok);
    resp->setStatusMessage("OK");
    resp->setContentType("text/html");
    resp->addHeader("Content-Length", std::to_string(entry.file_size()));
    resp->setBody(std::move(body));
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