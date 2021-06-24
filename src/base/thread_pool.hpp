/*
Copyright (c) 2012 Jakob Progsch, Václav Zeman

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
distribution.
*/

#ifndef _HTTPSERVER_SRC_BASE_THREAD_POOL_H__
#define _HTTPSERVER_SRC_BASE_THREAD_POOL_H__

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <future>
#include <type_traits>

class thread_pool{
public:
    explicit thread_pool(std::size_t size);

    template<typename Func, typename... Args>
    auto addTask(Func&& func, Args... args)
        ->std::future<std::result_of_t<Func(Args...)>>;

    void stop();
    ~thread_pool();
private:
    bool stop_{false};

    std::vector<std::thread> threads{};
    std::queue<std::function<void()>> tasks{};
    std::mutex mutex_;
    std::condition_variable cond_;
};


thread_pool::thread_pool(std::size_t size){
    for(std::size_t i = 0; i < size; ++i){
        threads.emplace_back(
        [this]{
            while(!stop){
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    cond_.wait(lock, [this](){return !tasks.empty() || stop_;});
                    if(stop_ && tasks.empty())    
                        return;
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        });
    }
}

template<typename Func, typename... Args>
auto thread_pool::addTask(Func&& func, Args... args)
    ->std::future<std::result_of_t<Func(Args...)>>
{
    using returnType = std::result_of_t<Func(Args...)>;

    auto task = std::make_shared<std::packaged_task<returnType()>>(
        std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
    );
    
    returnType ret = task->get_future();

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if(stop_)
            throw std::runtime_error{"add task to a stopped thread pool"};

        tasks.emplace([task](){(*task)();});
    }
    
    cond_.notify_one();  

    return ret;
}

void thread_pool::stop() {
    stop_ = true;
    cond_.notify_all();
}

thread_pool::~thread_pool(){
    {
        std::unique_lock<std::mutex> lock(mutex_);
        stop_ = true;
    }
    cond_.notify_all();
    
    for(auto& thread : threads){
        thread.join();
    }
}

#endif