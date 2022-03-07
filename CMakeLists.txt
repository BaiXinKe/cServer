cmake_minimum_required(VERSION 3.0.0)
project(cServer VERSION 0.1.0)

find_package(Threads REQUIRED)

add_compile_options(-g -rdynamic -Wall -Wextra -Wpedantic -std=c++17)

add_executable(${PROJECT_NAME} src/main.cc src/net/ThreadPool.cpp)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)
