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
