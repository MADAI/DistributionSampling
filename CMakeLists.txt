project( DistributionSampling )
cmake_minimum_required(VERSION 2.8)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMake")

set( DistributionSamplingLibrary_VERSION_MAJOR 0 )
set( DistributionSamplingLibrary_VERSION_MINOR 1 )
set( DistributionSamplingLibrary_VERSION_REVISION 0 )

set( DistributionSampling_INCLUDE_DIRS ${DistributionSampling_SOURCE_DIR}/src )
set( DistributionSampling_LIBRARY_DIRS ${DistributionSampling_BINARY_DIR}/lib )

list(APPEND DistributionSampling_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src )
list(APPEND DistributionSampling_LIBRARY_DIRS ${PROJECT_BINARY_DIR}/lib )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DistributionSampling_BINARY_DIR}/bin )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${DistributionSampling_BINARY_DIR}/lib )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY} )

## EIGEN
find_package(Eigen3 REQUIRED)
list(APPEND DistributionSampling_INCLUDE_DIRS ${EIGEN3_INCLUDE_DIR} )

## BOOST
find_package( Boost REQUIRED )
list(APPEND DistributionSampling_INCLUDE_DIRS ${Boost_INCLUDE_DIRS} )

## Compile time checks for the existence of required headers
set( try_compile_directory ${DistributionSampling_BINARY_DIR}/TryCompiles )
try_compile( HAS_POSIX_PIPES ${try_compile_directory}
  ${DistributionSampling_SOURCE_DIR}/CMake/HasPosixPipes.cxx
  OUTPUT_VARIABLE HAS_POSIX_PIPES_COMPILE_OUTPUT
)

if ( HAS_POSIX_PIPES )
  message( STATUS "posix pipes found" )
else()
  message( FATAL_ERROR "No posix pipes are available on this system!" )
  message( ${HAS_POSIX_PIPES_COMPILE_OUTPUT} )
endif()

## Distribution Library configuration file
configure_file( src/Configuration.h.in ${DistributionSampling_BINARY_DIR}/Configuration.h )
list(APPEND DistributionSampling_INCLUDE_DIRS ${DistributionSampling_BINARY_DIR} )
##

## include and link
include_directories( ${DistributionSampling_INCLUDE_DIRS})
link_directories( ${DistributionSampling_LIBRARY_DIRS} )

set( DistributionSampling_LIBRARIES  DistributionSampling)

add_subdirectory( src )

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

# Configure package file
configure_file(
  DistributionSamplingConfig.cmake.in
  ${PROJECT_BINARY_DIR}/DistributionSamplingConfig.cmake
  @ONLY )
