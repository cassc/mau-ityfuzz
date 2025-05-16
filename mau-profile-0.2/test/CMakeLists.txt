project(test LANGUAGES C CXX)
cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_COMPILER             "clang++")
set(CMAKE_CXX_STANDARD 11)

message(STATUS "CUDAPATH = ${CUDAPATH}")
include_directories(${CUDAPATH}/include/)
link_directories(${CUDAPATH}/lib64/)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/jsoncpp.cmake)
include_directories(JSONCPP_INCLUDE_DIR)

add_executable(test-runner runner-test.cc ../runner/runner.h ../runner/type.h debug.h)
target_link_libraries(test-runner cuda cudart runner jsoncpp)

set(SYSTEM_LIBS stdc++ ncurses z pthread dl m)
