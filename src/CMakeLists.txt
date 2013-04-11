set( SRC_FILES
  Random.cxx
  GaussianProcessEmulator.cxx
  GaussianProcessEmulatedModel.cxx
  Parameter.cxx
  Distribution.cxx
  ExternalModel.cxx
  GaussianDistribution.cxx
  LatinHypercubeGenerator.cxx
  Model.cxx
  Sampler.cxx
  Paths.cxx
  ProcessPipe.c
  RegularStepGradientAscentSampler.cxx
  Sample.cxx
  MetropolisHastingsSampler.cxx
  Trace.cxx
  UniformDistribution.cxx
)

add_library( DistributionSampling ${SRC_FILES} )
target_link_libraries( DistributionSampling madaisys )
