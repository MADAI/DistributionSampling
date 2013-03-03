set( LIBRARIES 
  DistributionSampling
  ${CORELIBS} 
)

set( TEST_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR} )

#add_executable( DiceMCMC DiceMCMC.cxx )
#target_link_libraries( DiceMCMC ${LIBRARIES} )
#add_test( DiceMCMC DiceMCMC ${TEST_BASE_DIR}/DiceMCMC )

add_executable( ExternalModelMCMCTest ExternalModelMCMCTest.cxx )
target_link_libraries( ExternalModelMCMCTest ${LIBRARIES} )
add_test( ExternalModelMCMCTest
  ExternalModelMCMCTest ${TEST_BASE_DIR}/ExternalModelMCMCTest/ExampleProcess.py )

#add_executable( Gaussian2DMCMCTest Gaussian2DMCMCTest.cxx )
#target_link_libraries( Gaussian2DMCMCTest ${LIBRARIES} )
#add_test( Gaussian2DMCMCTest Gaussian2DMCMCTest ${TEST_BASE_DIR}/Gaussian2DMCMC )

add_executable( Gaussian2DModelTest Gaussian2DModelTest.cxx )
target_link_libraries( Gaussian2DModelTest ${LIBRARIES} )
add_test( Gaussian2DModelTest Gaussian2DModelTest )

#add_executable( MarkovChainMonteCarloSamplerTest MarkovChainMonteCarloSamplerTest.cxx )
#target_link_libraries( MarkovChainMonteCarloSamplerTest ${LIBRARIES} )
#add_test( MarkovChainMonteCarloSamplerTest MarkovChainMonteCarloSamplerTest
#  ${TEST_BASE_DIR}/TestModel
#  ${TEST_BASE_DIR}/MarkovChainMonteCarloSamplerTest/MarkovChainMonteCarloSamplerTest.csv )

add_executable( ModelTest ModelTest.cxx )
target_link_libraries( ModelTest ${LIBRARIES} )
add_test( ModelTest ModelTest )

add_executable( NumericalGradientEstimationTest NumericalGradientEstimationTest.cxx )
target_link_libraries( NumericalGradientEstimationTest ${LIBRARIES} )
add_test( NumericalGradientEstimationTest NumericalGradientEstimationTest )

add_executable( RegularStepGradientDescentOptimizerTest RegularStepGradientDescentOptimizerTest.cxx )
target_link_libraries( RegularStepGradientDescentOptimizerTest ${LIBRARIES} )
add_test( RegularStepGradientDescentOptimizerTest RegularStepGradientDescentOptimizerTest )

add_executable( TraceTest TraceTest.cxx )
target_link_libraries( TraceTest ${LIBRARIES} )
add_test( TraceTest TraceTest )

add_executable( TraceElementTest TraceElementTest.cxx )
target_link_libraries( TraceElementTest ${LIBRARIES} )
add_test( TraceElementTest TraceElementTest )
