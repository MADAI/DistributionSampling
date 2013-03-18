set( SRC_FILES
  Random.cxx
  GaussianProcessEmulator.cxx
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

add_executable(
  printThetas
  printThetas.cxx)
target_link_libraries(
  printThetas
  DistributionSampling)
