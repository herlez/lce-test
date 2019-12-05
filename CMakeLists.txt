cmake_minimum_required (VERSION 3.5)
project (lceTest)

# configure a header file to pass some of the CMake settings
# to the source code

set(CMAKE_CXX_FLAGS " -Wall -std=c++17 -O3")

# include ferrada's rmq implementation
add_library(ferrada_rmq libs/rmq/RMQRMM64.cpp libs/rmq/includes/Basic_rmq.cpp)

target_include_directories(ferrada_rmq PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs>
  $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs>
)


# include command line parser from tlx
add_library(tlx_command_line
  libs/tlx/tlx/cmdline_parser.cpp
  libs/tlx/tlx/string/parse_si_iec_units.cpp)

target_include_directories(tlx_command_line PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/tlx/>
  $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/tlx/>
)

# Add sais-lite-lcp
add_library(sais_lite_lcp
  libs/sais-lite-lcp/sais.c)

target_include_directories(sais_lite_lcp PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/sais-lite-lcp>
  $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/sais-lite-lcp>
)

add_executable(bench_time ./src/bench_time.cpp)
target_link_libraries(bench_time ferrada_rmq stdc++fs tlx_command_line sais_lite_lcp)

#add_executable(testCorrectness ./src/test_correctness.cpp)
