################################################################################
# CMakeLists.txt
#
# Copyright (C) 2019 Alexander Herlez <alexander.herlez@tu-dortmund.de>
# Copyright (C) 2019 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
# Copyright (C) 2022 Patrick Dinklage <patrick.dinklage@tu-dortmund.de>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

cmake_minimum_required (VERSION 3.5)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project (lce-test)


option(DETAILED_TIME "Get detailed timing and memory output of SSS construction. This invalidates other memory mesurements." OFF)
if (DETAILED_TIME)
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -DDETAILED_TIME")
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE
  "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ffast-math -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb -DDEBUG")

# find OpenMP
option(ALLOW_PARALLEL "Allow use of parallel algorithms" ON)
if (ALLOW_PARALLEL)
  find_package(OpenMP)
  if (OPENMP_FOUND)
      set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -DALLOW_PARALLEL")
      set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
  endif()
endif()

# include ferrada's rmq implementation
add_library(ferrada_rmq
  extlib/rmq/RMQRMM64.cpp
  extlib/rmq/includes/Basic_rmq.cpp)

target_include_directories(ferrada_rmq PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extlib/rmq>
  $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extlib/rmq>
)

# include command line parser from tlx
add_library(tlx
  extlib/tlx/tlx/cmdline_parser.cpp
  extlib/tlx/tlx/string/parse_si_iec_units.cpp
  extlib/tlx/tlx/thread_pool.cpp
  extlib/tlx/tlx/multi_timer.cpp
  extlib/tlx/tlx/logger/core.cpp
  extlib/tlx/tlx/die/core.cpp
  )

target_include_directories(tlx PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extlib/tlx/>
  $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extlib/tlx/>
)

# include reworked version of sais_lite
add_library(sais_lcp
  extlib/sais-lite-lcp/sais.c
)
target_include_directories(sais_lcp PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extlib/sais-lite-lcp/>
  $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extlib/sais-lite-lcp/>
)

# include parallel-sais
add_library(libsais
  extlib/libsais/src/libsais.c
)
target_include_directories(libsais PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extlib/libsais/>
  $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extlib/libsais/>
)

# Change this line to your tbb path
list(APPEND CMAKE_PREFIX_PATH /work/smarherl/tbb/lib64/cmake/TBB)
find_package(TBB REQUIRED)
add_subdirectory(benchmark)
#set(IPS4O_USE_OPENMP TRUE)
#add_subdirectory(extlib/ips4o)

# include PGM index
add_library(pgm_index INTERFACE)
target_include_directories(pgm_index INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extlib/PGM-index/include/>
  $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extlib/PGM-index/include/>
)

################################################################################
