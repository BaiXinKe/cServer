#include "net/EventLoop.hpp"
#include "net/TcpServer.hpp"
#include <iostream>

void messageCallback(const Duty::TcpConnectionPtr& conn, Duty::Buffer* buf, Duty::Timestamp ts)
{
    (void)conn;
    const char* line { nullptr };
    while ((line = buf->findCRLF()) != nullptr) {
        std::string lineContent { buf->Peek(), line - buf->Peek() };
        buf->retrieve(line - buf->Peek() + 2);

        auto sec = std::chrono::system_clock::to_time_t(ts);
        std::string timeString { ctime(&sec) };

        std::cout << lineContent << "\r\n";
    }
}

int main()
{
    Duty::EventLoop mainLoop;

    Duty::InetAddr local { "0.0.0.0", 1234 };

    Duty::TcpServer server(&mainLoop, local, "mainServer");

    server.setMessageCallback(messageCallback);

    server.setThreadNum(0);

    server.start();

    mainLoop.loop();

    getchar();
}