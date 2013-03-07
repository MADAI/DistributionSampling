project( DistributionSampling )
cmake_minimum_required(VERSION 2.8)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMake")

set( DistributionSampling_INCLUDE_DIRS ${DistributionSampling_SOURCE_DIR}/src )
set( DistributionSampling_LIBRARY_DIRS ${DistributionSampling_BINARY_DIR}/lib )

## EIGEN
find_package(Eigen3 REQUIRED)
list(APPEND DistributionSampling_INCLUDE_DIRS ${EIGEN3_INCLUDE_DIR} )

## GSL
find_package( GSL REQUIRED )
list(APPEND CORELIBS ${GSL_LIBRARIES} )
list(APPEND DistributionSampling_INCLUDE_DIRS ${GSL_INCLUDE_DIRS} )
list(APPEND DistributionSampling_LIBRARY_DIRS ${GSL_LIBRARY_DIRS} )

## MADAIEmulator
find_package( MADAIEmulator REQUIRED )
list(APPEND CORELIBS ${MADAIEmulator_LIBRARIES} )
list(APPEND DistributionSampling_INCLUDE_DIRS ${MADAIEmulator_INCLUDE_DIRS} )
list(APPEND DistributionSampling_LIBRARY_DIRS ${MADAIEmulator_LIBRARY_DIRS} )

##

include_directories( ${DistributionSampling_INCLUDE_DIRS})
link_directories( ${DistributionSampling_LIBRARY_DIRS} )
set( DistributionSampling_LIBRARIES  DistributionSampling  ${CORELIBS})

add_subdirectory( src )

option( BUILD_TESTING "Build the tests for DistributionSampling." OFF )
if ( BUILD_TESTING )
  enable_testing()
  add_subdirectory( test )
endif()

# Configure package file
configure_file( DistributionSamplingConfig.cmake.in ${PROJECT_BINARY_DIR}/DistributionSamplingConfig.cmake @ONLY )
