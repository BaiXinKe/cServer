cmake_minimum_required(VERSION 3.0.0)
project(httpServer VERSION 0.1.0)

include(CTest)
enable_testing()

aux_source_directory(src MAIN_LIB)

add_executable(httpServer ${MAIN_LIB})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
