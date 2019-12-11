################################################################################
# CMakeLists.txt
#
# Copyright (C) 2019 Alexander Herlez <alexander.herlez@tu-dortmund.de>
# Copyright (C) 2019 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

# include malloc_count
add_library(malloc_count
  ${PROJECT_SOURCE_DIR}/extlib/malloc_count/malloc_count.c
  ${PROJECT_SOURCE_DIR}/extlib/malloc_count/stack_count.c
)
  
target_include_directories(malloc_count PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/extlib/malloc_count/>
  $<INSTALL_INTERFACE:${PROJECT_SOURCE_DIR}/extlib/malloc_count/>
)

add_executable(bench_time
  bench_time.cpp)

target_include_directories(bench_time PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lce-test/>
  $<INSTALL_INTERFACE:${PROJECT_SOURCE_DIR}/lce-test/>
)

target_link_libraries(bench_time ferrada_rmq tlx stdc++fs sais_lcp)

# add_executable(bench_memory ./src/bench_memory.cpp)
# target_link_libraries(bench_memory ferrada_rmq tlx_command_line sais_lcp)
# target_link_libraries(bench_memory malloc_count -ldl)

################################################################################
