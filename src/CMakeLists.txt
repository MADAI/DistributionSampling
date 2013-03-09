set( SRC_FILES
  Random.cxx
  GaussianProcessModelEmulator.cxx
  GaussianProcessEmulatedModel.cxx
  Parameter.cxx
  Distribution.cxx
  ExternalModel.cxx
  GaussianDistribution.cxx
  Model.cxx
  Sampler.cxx
  ProcessPipe.c
  RegularStepGradientAscentSampler.cxx
  Sample.cxx
  SimpleMetropolisHastingsSampler.cxx
  Trace.cxx
  UniformDistribution.cxx
)

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${DistributionSampling_BINARY_DIR}/lib )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY} )
add_library( DistributionSampling ${SRC_FILES} )

add_executable(
  emulate
  emulate.cxx )
target_link_libraries(
  emulate
  DistributionSampling)

add_executable(
  trainEmulator
  trainEmulator.cxx)
target_link_libraries(
  trainEmulator
  DistributionSampling)

add_executable(
  basicTrain
  basicTrain.cxx)
target_link_libraries(
  basicTrain
  DistributionSampling)
