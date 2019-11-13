cmake_minimum_required (VERSION 3.5)
project (lceTest)

# configure a header file to pass some of the CMake settings
# to the source code
  
set(CMAKE_CXX_FLAGS " -Wall -std=c++17 -O3 -ftemplate-backtrace-limit=0")

#LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/res)
#set(CMAKE_CXX_FLAGS "-O0 -ggdb") 
add_executable(bench_time ./src/bench_time.cpp)
#add_library(bench_time ./src/bench_time.cpp)
target_link_libraries (bench_time ${CMAKE_SOURCE_DIR}/libs/rmqrmmBP.a)

#add_executable(testCorrectness ./src/test_correctness.cpp)

