cmake_minimum_required (VERSION 2.6)
project (lceTest)

# configure a header file to pass some of the CMake settings
# to the source code
  
#set(CMAKE_CXX_FLAGS "-O0 -ggdb")  
set(CMAKE_CXX_FLAGS " -Wall -std=c++17 -fno-exceptions -O3 -ftemplate-backtrace-limit=0")
add_executable(testTime ./src/testTime.cpp)
add_executable(testSss ./src/testSss.cpp)
#set(CMAKE_CXX_FLAGS "-O0 -ggdb")  
add_executable(testCorrectness ./src/testCorrectness.cpp)
