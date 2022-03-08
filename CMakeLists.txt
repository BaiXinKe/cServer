cmake_minimum_required(VERSION 3.0.0)
project(cServer VERSION 0.1.0)

find_package(Threads REQUIRED)

add_compile_options(-g -rdynamic -Wall -Wextra -Wpedantic -std=c++17)

set(SOURCE 
    src/net/Buffer.cpp
    src/net/Channel.cpp
    src/net/Error.cpp
    src/net/EventLoop.cpp
    src/net/EventLoopThread.cpp
    src/net/EventLoopThreadPool.cpp 
    src/net/InetAddr.cpp
    src/net/ListenSocket.cpp 
    src/net/Poller.cpp 
    src/net/Socket.cpp
    src/net/TcpConnection.cpp
    src/net/Timer.cpp 
    src/net/TimerId.cpp
    src/net/TimerQueue.cpp 
    src/net/WakeUp.cpp
    src/net/TcpServer.cpp)

add_executable(${PROJECT_NAME} src/main.cc ${SOURCE})

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)
