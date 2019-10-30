cmake_minimum_required (VERSION 2.6)
project (lceTest)

# configure a header file to pass some of the CMake settings
# to the source code
  
set(CMAKE_CXX_FLAGS " -Wall -std=c++17 -O3 -ftemplate-backtrace-limit=0")
#set(CMAKE_CXX_FLAGS "-O0 -ggdb")  
add_executable(bench_time ./src/bench_time.cpp)
#add_executable(testCorrectness ./src/test_correctness.cpp)
