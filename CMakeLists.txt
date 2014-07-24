project( DistributionSampling )
cmake_minimum_required(VERSION 2.6)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMake")

set( DistributionSamplingLibrary_VERSION_MAJOR 0 )
set( DistributionSamplingLibrary_VERSION_MINOR 1 )
set( DistributionSamplingLibrary_VERSION_REVISION 0 )
set( DistributionSamplingLibrary_VERSION_STRING
  "${DistributionSamplingLibrary_VERSION_MAJOR}.${DistributionSamplingLibrary_VERSION_MINOR}.${DistributionSamplingLibrary_VERSION_REVISION}" )

set( DistributionSampling_INCLUDE_DIRS ${DistributionSampling_SOURCE_DIR}/src )
set( DistributionSampling_LIBRARY_DIRS ${DistributionSampling_BINARY_DIR}/lib )

list(APPEND DistributionSampling_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src )
list(APPEND DistributionSampling_LIBRARY_DIRS ${PROJECT_BINARY_DIR}/lib )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DistributionSampling_BINARY_DIR}/bin )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${DistributionSampling_BINARY_DIR}/lib )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY} )

## Turn on all standard compiler warnings
if(CMAKE_COMPILER_IS_GNUCXX OR (CMAKE_CXX_COMPILER MATCHES ".*clang"))
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-long-long")
endif()

## For checking that developers don't introduce warnings.
option (ABORT_ON_COMPILER_WARNINGS "With GCC, use the -Werror option." OFF)
if((ABORT_ON_COMPILER_WARNINGS) AND
    (CMAKE_COMPILER_IS_GNUCXX OR (CMAKE_CXX_COMPILER MATCHES ".*clang")))
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif()

## EIGEN
find_package(Eigen3 REQUIRED)
list(APPEND DistributionSampling_INCLUDE_DIRS ${EIGEN3_INCLUDE_DIR} )

## BOOST
find_package( Boost REQUIRED iostreams)
list(APPEND DistributionSampling_INCLUDE_DIRS ${Boost_INCLUDE_DIRS} )

## OpenMP
option (USE_OPENMP "Compile with OpenMP support." OFF)
if(USE_OPENMP)
  find_package( OpenMP ) # not REQUIRED
  if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
  endif() # OPENMP_FOUND
endif() # USE_OPENMP

## gprof (default off)
option (USE_GPROF "Compile with gprof profiler." OFF)
if(USE_GPROF)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
endif() # USE_GPROF

option( BUILD_DOCUMENTATION "Build Doxygen documentation." OFF )

if ( BUILD_DOCUMENTATION )
  find_package( Doxygen )
  if ( DOXYGEN_FOUND )
    add_subdirectory( doc )
  endif()
endif()

## Distribution Library configuration file
configure_file( src/Configuration.h.in ${DistributionSampling_BINARY_DIR}/Configuration.h )
list(APPEND DistributionSampling_INCLUDE_DIRS ${DistributionSampling_BINARY_DIR} )
install( FILES ${DistributionSampling_BINARY_DIR}/Configuration.h
  DESTINATION include/madai/DistributionSampling
)
##

## include and link
include_directories( ${DistributionSampling_INCLUDE_DIRS}
  ${DistributionSampling_BINARY_DIR}/ThirdParty/KWSys/)
link_directories( ${DistributionSampling_LIBRARY_DIRS} )

set( DistributionSampling_LIBRARIES  DistributionSampling)

option( BUILD_TESTING "Build the tests for DistributionSampling." OFF )
if ( BUILD_TESTING )
  enable_testing()
  add_subdirectory( test )

  set( DistributionSampling_TEST_INCLUDE_DIRS
    ${DistributionSampling_SOURCE_DIR}/test )
  set( DistributionSampling_TEST_LIBRARY_DIRS
    ${DistributionSampling_BINARY_DIR}/test )
  set( DistributionSampling_TEST_LIBRARIES DistributionSamplingTest )

endif()

add_subdirectory( applications )
add_subdirectory( src )
add_subdirectory( ThirdParty )

# Configure package file
configure_file(
  DistributionSamplingConfig.cmake.in
  ${PROJECT_BINARY_DIR}/DistributionSamplingConfig.cmake
  @ONLY )
