set( LIBRARIES 
  ModelOptimization
  ${CORELIBS} 
)

set( TEST_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( Gaussian2DModelTest Gaussian2DModelTest.cxx )
target_link_libraries( Gaussian2DModelTest ${LIBRARIES} )
add_test( Gaussian2DModelTest Gaussian2DModelTest )

add_executable( ModelTest ModelTest.cxx )
target_link_libraries( ModelTest ${LIBRARIES} )
add_test( ModelTest ModelTest )

add_executable( NumericalGradientEstimationTest NumericalGradientEstimationTest.cxx )
target_link_libraries( NumericalGradientEstimationTest ${LIBRARIES} )
add_test( NumericalGradientEstimationTest NumericalGradientEstimationTest )

add_executable( RegularStepGradientDescentOptimizerTest RegularStepGradientDescentOptimizerTest.cxx )
target_link_libraries( RegularStepGradientDescentOptimizerTest ${LIBRARIES} )
add_test( RegularStepGradientDescentOptimizerTest RegularStepGradientDescentOptimizerTest )

add_executable( external_model_plus_mhmcmc_test external_model_plus_mhmcmc_test.cxx )
target_link_libraries( external_model_plus_mhmcmc_test ${LIBRARIES} )
add_test( external_model_plus_mhmcmc_test external_model_plus_mhmcmc_test )

add_executable( Gaussian2DMCMCTest Gaussian2DMCMCTest.cxx )
target_link_libraries( Gaussian2DMCMCTest ${LIBRARIES} )
add_test( Gaussian2DMCMCTest Gaussian2DMCMCTest ${TEST_BASE_DIR}/Gaussian2DMCMC )

add_executable( MCMC_core_test MCMC_core_test.cxx )
target_link_libraries( MCMC_core_test ${LIBRARIES} )
add_test( MCMC_core_test MCMC_core_test )

add_executable( DiceMCMC DiceMCMC.cxx )
target_link_libraries( DiceMCMC ${LIBRARIES} )
add_test( DiceMCMC DiceMCMC ${TEST_BASE_DIR}/DiceMCMC )
