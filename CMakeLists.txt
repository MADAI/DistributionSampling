project(ModelOptimization)
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
)

link_directories(
  ${PROJECT_BINARY_DIR}/lib
  ${coral_LIBRARY_DIRS}
  ${MADAIEmulator_LIBRARY_DIRS}
)

add_subdirectory( src )
add_subdirectory( main )

set( ModelOptimization_INCLUDE_DIRS
  ${ModelOptimization_SOURCE_DIR}/include
  ${coral_INCLUDE_DIRS}
  ${MADAIEmulator_INCLUDE_DIRS}
)

set( ModelOptimization_LIBRARY_DIRS
  ${ModelOptimization_BINARY_DIR}/lib
  ${coral_LIBRARY_DIRS}
  ${MADAIEmulator_LIBRARY_DIRS}
)

set( ModelOptimization_LIBRARIES
  ModelOptimization
  ${coral_LIBRARIES}
  ${MADAIEmulator_LIBRARIES}
)

option( BUILD_TESTING "Build the tests for ModelOptimization." OFF )
if ( BUILD_TESTING )
  enable_testing()
  add_subdirectory( test )
endif()

# Configure package file
configure_file( ModelOptimizationConfig.cmake.in ${PROJECT_BINARY_DIR}/ModelOptimizationConfig.cmake @ONLY )
