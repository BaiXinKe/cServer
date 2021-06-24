cmake_minimum_required(VERSION 3.0.0)
project(httpServer VERSION 0.1.0)

include(CTest)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE src/main.cpp)


add_executable(httpServer ${SOURCE})

target_include_directories(httpServer
    PRIVATE ${PROJECT_SOURCE_DIR}/include)



set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
