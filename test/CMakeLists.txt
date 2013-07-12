set( LIBRARIES
  DistributionSampling
  ${CORELIBS}
)

set( TEST_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR} )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ )

include_directories( ${DistributionSampling_SOURCE_DIR}/applications )

add_library( DistributionSamplingTest STATIC
  Gaussian2DModel.cxx
)
target_link_libraries( DistributionSamplingTest ${LIBRARIES} )

foreach( test
  LangevinSamplerTest
  Gaussian2DModelTest
  MetropolisHastingsSamplerTest
  NumericalGradientEstimationTest
  PercentileGridSamplerTest
  RegularStepGradientAscentSamplerTest
  )
  add_executable( ${test} ${test}.cxx )
  target_link_libraries( ${test} ${LIBRARIES} DistributionSamplingTest )
  add_test( ${test} ${test} )
endforeach()

foreach( test
  GaussianDistributionTest
  LatinHypercubeGeneratorTest
  ModelTest
  PrincipalComponentDecomposeTest
  RandomTest
  RetainPrincipalComponentsTest
  RuntimeParameterFileReaderTest
  SampleTest
  UniformDistributionTest
  )
  add_executable( ${test} ${test}.cxx )
  target_link_libraries( ${test} ${LIBRARIES} )
  add_test( ${test} ${test} )
endforeach()

add_executable( ExternalModelMCMCTest ExternalModelMCMCTest.cxx )
target_link_libraries( ExternalModelMCMCTest ${LIBRARIES} )
add_test( ExternalModelMCMCTest
  ExternalModelMCMCTest ${TEST_BASE_DIR}/ExternalModelMCMCTest/ExampleProcess.py )

#add_executable( Gaussian2DMCMCTest Gaussian2DMCMCTest.cxx )
#target_link_libraries( Gaussian2DMCMCTest ${LIBRARIES} )
#add_test( Gaussian2DMCMCTest Gaussian2DMCMCTest ${TEST_BASE_DIR}/Gaussian2DMCMC )

add_executable( GaussianProcessEmulatorTest
  GaussianProcessEmulatorTest.cxx
  GaussianProcessEmulatorTestGenerator.cxx )
target_link_libraries( GaussianProcessEmulatorTest ${LIBRARIES} )
add_test( GaussianProcessEmulatorTest GaussianProcessEmulatorTest )

add_executable( GaussianProcessEmulatedModelTest
  GaussianProcessEmulatedModelTest.cxx
  GaussianProcessEmulatorTestGenerator.cxx )
target_link_libraries( GaussianProcessEmulatedModelTest ${LIBRARIES} )
add_test( GaussianProcessEmulatedModelTest GaussianProcessEmulatedModelTest )

add_executable( GaussianProcessEmulatorDirectoryReaderTest
  GaussianProcessEmulatorDirectoryReaderTest.cxx )
target_link_libraries( GaussianProcessEmulatorDirectoryReaderTest
  ${LIBRARIES}
  ApplicationUtilities
)
add_test( GaussianProcessEmulatorDirectoryReaderTest
  GaussianProcessEmulatorDirectoryReaderTest
  ${TEST_BASE_DIR}/DirectoryStructureTest/statistical_analysis
)

#add_executable( GradientOutputsTest
#  GradientOutputsTest.cxx
#  GaussianProcessEmulatorTestGenerator.cxx )
#target_link_libraries( GradientOutputsTest ${LIBRARIES} ApplicationUtilities )
#add_test( GradientOutputsTest GradientOutputsTest )
