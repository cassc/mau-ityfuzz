project(runner LANGUAGES C CXX)
cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_COMPILER             "clang++")
set(CMAKE_CXX_STANDARD 11)

# include_directories("/usr/local/cuda-11.4/include/")

# if(CUDAPATH)
message(STATUS "CUDAPATH = ${CUDAPATH}")
include_directories(${CUDAPATH}/include/)
link_directories(${CUDAPATH}/lib64/)

# else()
#   include_directories("/usr/local/cuda-11.4/include/")
# endif()

#include_directories("/usr/lib/x86_64-linux-gnu/libcuda.so")
set(SOURCES_LIBS
  ./type.h
  ./runner.cc
  ./debug.h
  ./debug.cc
  ./fuzzer_config.h
)

add_compile_options(-fms-extensions)

add_library(runner SHARED ${SOURCES_LIBS})

target_link_libraries(runner cuda cudart)


# add_executable(test-runner test.cc runner.h type.h)
# target_link_libraries(test-runner cuda cudart)

set(SYSTEM_LIBS stdc++ ncurses z pthread dl m)
