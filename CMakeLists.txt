project( DistributionSampling )
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMake")

find_package( coral REQUIRED )
find_package( MADAIEmulator REQUIRED )
find_package( GSL REQUIRED )
set(CORELIBS
  ${coral_LIBRARIES}
  ${MADAIEmulator_LIBRARIES}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${coral_INCLUDE_DIRS}
  ${MADAIEmulator_INCLUDE_DIRS}
)

link_directories(
  ${PROJECT_BINARY_DIR}/lib
  ${coral_LIBRARY_DIRS}
  ${MADAIEmulator_LIBRARY_DIRS}
)

add_subdirectory( src )

set( DistributionSampling_INCLUDE_DIRS
  ${DistributionSampling_SOURCE_DIR}/src
  ${coral_INCLUDE_DIRS}
  ${MADAIEmulator_INCLUDE_DIRS}
)

set( DistributionSampling_LIBRARY_DIRS
  ${DistributionSampling_BINARY_DIR}/lib
  ${coral_LIBRARY_DIRS}
  ${MADAIEmulator_LIBRARY_DIRS}
)

set( DistributionSampling_LIBRARIES
  DistributionSampling
  ${coral_LIBRARIES}
  ${MADAIEmulator_LIBRARIES}
)

option( BUILD_TESTING "Build the tests for DistributionSampling." OFF )
if ( BUILD_TESTING )
  enable_testing()
  add_subdirectory( test )
endif()

# Configure package file
configure_file( DistributionSamplingConfig.cmake.in ${PROJECT_BINARY_DIR}/DistributionSamplingConfig.cmake @ONLY )
