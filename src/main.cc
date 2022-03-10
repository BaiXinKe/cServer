#include "net/EventLoop.hpp"
#include "net/HttpServer.hpp"
#include "net/TcpServer.hpp"
#include <iostream>

int main()
{
    try {
        Duty::EventLoop mainLoop;
        Duty::InetAddr addr { "0.0.0.0", 1234 };

        Duty::HttpServer server(&mainLoop, addr, "mainHttpServer");
        server.setThreadNum();
        server.start();

        mainLoop.loop();
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }
}