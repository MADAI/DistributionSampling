set( LIBRARIES
  DistributionSampling
  ApplicationUtilities
  ${CORELIBS}
)

# Put the tests in test subdirectory
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DistributionSampling_BINARY_DIR}/applications/test )

include_directories( ${DistributionSampling_SOURCE_DIR}/applications )

foreach( test
  EnsurePathSeparatorAtEndTest
  FindIndexTest
  LowerCaseTest
  SplitStringTest
  )
  add_executable( ${test} ${test}.cxx )
  target_link_libraries( ${test} ${LIBRARIES} )
  add_test( ${test} ${test} )
endforeach()
