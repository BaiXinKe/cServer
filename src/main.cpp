#include <future>
#include <iostream>
#include "base/ThreadPool.hpp"

void func()
{
    std::cout << "Hello world" << std::endl;
}

int main(int, char **)
{
    cServer::base::ThreadPool pool{};

    pool.submit(func);
}
