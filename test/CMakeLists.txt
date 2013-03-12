set( LIBRARIES
  DistributionSampling
  ${CORELIBS}
)

set( TEST_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( ExternalModelMCMCTest ExternalModelMCMCTest.cxx )
target_link_libraries( ExternalModelMCMCTest ${LIBRARIES} )
add_test( ExternalModelMCMCTest
  ExternalModelMCMCTest ${TEST_BASE_DIR}/ExternalModelMCMCTest/ExampleProcess.py )

#add_executable( Gaussian2DMCMCTest Gaussian2DMCMCTest.cxx )
#target_link_libraries( Gaussian2DMCMCTest ${LIBRARIES} )
#add_test( Gaussian2DMCMCTest Gaussian2DMCMCTest ${TEST_BASE_DIR}/Gaussian2DMCMC )

add_executable( GaussianDistributionTest GaussianDistributionTest.cxx )
target_link_libraries( GaussianDistributionTest ${LIBRARIES} )
add_test( GaussianDistributionTest GaussianDistributionTest )

add_executable( Gaussian2DModelTest Gaussian2DModelTest.cxx )
target_link_libraries( Gaussian2DModelTest ${LIBRARIES} )
add_test( Gaussian2DModelTest Gaussian2DModelTest )

add_executable( ModelTest ModelTest.cxx )
target_link_libraries( ModelTest ${LIBRARIES} )
add_test( ModelTest ModelTest )

add_executable( NumericalGradientEstimationTest NumericalGradientEstimationTest.cxx )
target_link_libraries( NumericalGradientEstimationTest ${LIBRARIES} )
add_test( NumericalGradientEstimationTest NumericalGradientEstimationTest )

add_executable( SimpleMetropolisHastingsSamplerTest SimpleMetropolisHastingsSamplerTest.cxx )
target_link_libraries( SimpleMetropolisHastingsSamplerTest ${LIBRARIES} )
add_test( SimpleMetropolisHastingsSamplerTest SimpleMetropolisHastingsSamplerTest )

add_executable( RandomTest RandomTest.cxx )
target_link_libraries( RandomTest ${LIBRARIES} )
add_test( RandomTest RandomTest )

add_executable( RegularStepGradientAscentSamplerTest RegularStepGradientAscentSamplerTest.cxx )
target_link_libraries( RegularStepGradientAscentSamplerTest ${LIBRARIES} )
add_test( RegularStepGradientAscentSamplerTest RegularStepGradientAscentSamplerTest )

add_executable( TraceTest TraceTest.cxx )
target_link_libraries( TraceTest ${LIBRARIES} )
add_test( TraceTest TraceTest )

add_executable( SampleTest SampleTest.cxx )
target_link_libraries( SampleTest ${LIBRARIES} )
add_test( SampleTest SampleTest )

add_executable( UniformDistributionTest UniformDistributionTest.cxx )
target_link_libraries( UniformDistributionTest ${LIBRARIES} )
add_test( UniformDistributionTest UniformDistributionTest )

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
