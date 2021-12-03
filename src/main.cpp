#include <future>
#include <iostream>
#include "base/TaskQueue.hpp"
#include "base/ThreadLocalQueue.hpp"

cServer::base::TaskQueue<std::packaged_task<int()>> taskQueue;

int task_func()
{
    std::cout << "Hello World";
    return 1;
}

void run_thread()
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    auto task = taskQueue.wait_pop();
    if (task != nullptr)
    {
        (*task)();
    }
}

int main(int, char **)
{
    std::packaged_task<int()> task(task_func);
    auto res = task.get_future();
    taskQueue.push(std::move(task));

    std::thread t{run_thread};
    t.join();

    std::cout << "This result is " << res.get() << '\n';
}
