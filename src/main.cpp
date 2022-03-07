#include "net/EventLoop.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

void func()
{
    std::cout << "Hello World\n";
}

int main(int, char**)
{
    EventLoop loop;

    loop.loop();
}
