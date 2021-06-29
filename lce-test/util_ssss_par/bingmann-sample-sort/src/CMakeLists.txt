################################################################################
# src/CMakeLists.txt
#
# Simple CMake file
################################################################################

include_directories("${PROJECT_SOURCE_DIR}/minitbb/")
include_directories("${PROJECT_BINARY_DIR}/")
include_directories("${GMP_INCLUDE_DIR}")
include_directories("${NUMA_INCLUDE_DIR}")

set(PSSLIB_SOURCES
  tools/globals.cpp
  sequential/bingmann-sample_sort.cpp
  sequential/bingmann-sample_sortBS.cpp
  sequential/bingmann-sample_sortBSC.cpp
  sequential/bingmann-sample_sortBT.cpp
  sequential/bingmann-sample_sortRBTCE.cpp
  parallel/bingmann-parallel_sample_sort.cpp
  )

set(PSS_LIBRARIES ${TBB_LIBRARIES} ${GMP_LIBRARIES} ${NUMA_LIBRARIES} tlx rt dl)

# build parallel string sorting static library from sources (possibly again)
if(PSS_STATIC)

  add_library(pss_static STATIC ${PSSLIB_SOURCES})
  target_link_libraries(pss_static ${PSS_LIBRARIES})

  set_target_properties(pss_static PROPERTIES
    VERSION   "${PSSLIB_VERSION}"
    SOVERSION "${PSSLIB_SOVERSION}")

endif()

# build parallel string sorting shared library from sources (possibly again)
if(PSS_SHARED)

  add_library(pss_shared SHARED ${PSSLIB_SOURCES})
  target_link_libraries(pss_shared ${PSS_LIBRARIES})

  set_target_properties(pss_shared PROPERTIES
    VERSION   "${PSSLIB_VERSION}"
    SOVERSION "${PSSLIB_SOVERSION}")

endif()

################################################################################
