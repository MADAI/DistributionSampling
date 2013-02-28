set( SRC_FILES
  GPEmulatedModel.cxx
  CosmoLikelihoodDistribution.cxx
  CosmoModel.cxx
  CosmoPriorDistribution.cxx
  DiceModel.cxx
  Distribution.cxx
  ExternalModel.cxx
  Gaussian2DModel.cxx
  LikelihoodDistribution.cxx
  MCMCRun.cxx
  Model.cxx
  MultiModel.cxx
  Sampler.cxx
  PriorDistribution.cxx
  ProcessPipe.c
  RHICModel.cxx
  RHIC_LikelihoodDistribution.cxx
  RHIC_PriorDistribution.cxx
  RegularStepGradientDescentSampler.cxx
  SimpleMetropolisHastingsSampler.cxx
  TestLikelihoodDistribution.cxx
  TestModel.cxx
  TestPriorDistribution.cxx
  Trace.cxx
  TraceElement.cxx
)

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${DistributionSampling_BINARY_DIR}/lib )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY} )
add_library( DistributionSampling ${SRC_FILES} )
