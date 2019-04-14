cmake_minimum_required (VERSION 2.6)
project (lceTest)

# configure a header file to pass some of the CMake settings
# to the source code
  
#set(CMAKE_CXX_FLAGS "-Wall -Werror -O3")  
set(CMAKE_CXX_FLAGS "-std=c++17 -O3 -fno-exceptions -ftemplate-backtrace-limit=0")
add_executable(testTime ./src/testTime.cpp)
add_executable(testCorrectness ./src/testCorrectness.cpp)
