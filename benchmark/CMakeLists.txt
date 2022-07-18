################################################################################
# CMakeLists.txt
#
# Copyright (C) 2019 Alexander Herlez <alexander.herlez@tu-dortmund.de>
# Copyright (C) 2019 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
# Copyright (C) 2022 Patrick Dinklage <patrick.dinklage@tu-dortmund.de>
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

target_compile_options(bench_time PRIVATE -Wall -Wextra -pedantic -O3) #-Winline -Werror

find_package(SDSL REQUIRED)
find_package(divsufsort REQUIRED)
find_package(divsufsort64 REQUIRED)

target_include_directories(bench_time PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lce-test/>
  $<INSTALL_INTERFACE:${PROJECT_SOURCE_DIR}/lce-test/>
  #/work/smarherl/oneapi-tbb-2021.3.0/include
  #${SDSL_INCLUDE_DIRS}
  #${PROJECT_SOURCE_DIR}/extlib/ips4o/include
)

target_include_directories(bench_time SYSTEM PRIVATE
  ${SDSL_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/extlib/ips4o/include
)

target_include_directories(bench_time SYSTEM PRIVATE
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/extlib/parallel-hashmap>
  $<INSTALL_INTERFACE:${PROJECT_SOURCE_DIR}/extlib/parallel-hashmap>
)

target_link_libraries(bench_time PRIVATE
  ferrada_rmq tlx sais_lcp libsais ${SDSL} ${divsufsort} ${divsufsort64} pgm_index malloc_count -ldl PRIVATE TBB::tbb)


option(LCE_BUILD_SDSL
  "Build the minimal benchmark based on the SDSL which uses CSTs to compute LCEs" OFF)

if (LCE_BUILD_SDSL)

  find_package(SDSL REQUIRED)
  find_package(divsufsort REQUIRED)
  find_package(divsufsort64 REQUIRED)

  add_executable(sdsl_cst_benchmark
    sdsl_cst_benchmark.cpp)
  target_link_libraries(sdsl_cst_benchmark ${SDSL} ${divsufsort} ${divsufsort64})

  target_include_directories(sdsl_cst_benchmark PUBLIC
    ${SDSL_INCLUDE_DIRS}
    $<BUILD_INTERFACE:${CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CURRENT_SOURCE_DIR}>
    )

endif()

add_executable(bench_sparse_ss bench_sparse_ss.cpp)

target_compile_options(bench_sparse_ss PRIVATE -Wall -Wextra -pedantic -O3) #-Winline -Werror

target_include_directories(bench_sparse_ss PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lce-test/>
  $<INSTALL_INTERFACE:${PROJECT_SOURCE_DIR}/lce-test/>
  #${SDSL_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/extlib/ips4o/include
  ${PROJECT_SOURCE_DIR}/extlib/parallel-hashmap
  ${PROJECT_SOURCE_DIR}/extlib/libsais
)


target_link_libraries(bench_sparse_ss PRIVATE tlx malloc_count -ldl libsais TBB::tbb)

add_executable(genqueries genqueries.cpp)
target_link_libraries(genqueries PRIVATE tlx)

add_executable(bench_predecessor bench_predecessor.cpp)
target_link_libraries(bench_predecessor PRIVATE pgm_index tlx malloc_count -ldl)

target_include_directories(bench_predecessor PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lce-test/>
  $<INSTALL_INTERFACE:${PROJECT_SOURCE_DIR}/lce-test/>
)

################################################################################
