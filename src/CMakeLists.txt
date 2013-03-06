set( SRC_FILES
  GaussianProcessEmulatedModel.cxx
  Distribution.cxx
  ExternalModel.cxx
  Gaussian2DModel.cxx
  MarkovChainMonteCarloSampler.cxx
  Model.cxx
  Sampler.cxx
  ProcessPipe.c
  RegularStepGradientDescentSampler.cxx
  SimpleMetropolisHastingsSampler.cxx
  Trace.cxx
  TraceElement.cxx
  UniformDistribution.cxx
)

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${DistributionSampling_BINARY_DIR}/lib )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY} )
add_library( DistributionSampling ${SRC_FILES} )
