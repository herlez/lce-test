cmake_minimum_required (VERSION 3.5)
project (lceTest)

# configure a header file to pass some of the CMake settings
# to the source code
set(CMAKE_CXX_FLAGS " -DNDEBUG -Wall -std=c++17 -O3")
set(CMAKE_C_FLAGS "-DNDEBUG -Wall -O3")

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

# include malloc_count
add_library(malloc_count
  libs/malloc_count/malloc_count.c
  libs/malloc_count/stack_count.c
)
  
target_include_directories(malloc_count PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/malloc_count/>
  $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/malloc_count/>
)

# include reworked version of sais_lite
add_library(sais_lcp
  libs/sais-lite-lcp/sais.c
)
target_include_directories(sais_lcp PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/sais-lite-lcp/>
  $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/sais-lite-lcp/>
)
  

add_executable(bench_time ./src/bench_time.cpp)
target_link_libraries(bench_time ferrada_rmq)
target_link_libraries(bench_time tlx_command_line stdc++fs)
target_link_libraries(bench_time sais_lcp)


# add_executable(bench_memory ./src/bench_memory.cpp)
# target_link_libraries(bench_memory ferrada_rmq tlx_command_line sais_lcp)
# target_link_libraries(bench_memory malloc_count -ldl)
