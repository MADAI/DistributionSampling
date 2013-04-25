set( SRC_FILES
  Random.cxx
  GaussianProcessEmulator.cxx
  GaussianProcessEmulatedModel.cxx
  GaussianProcessEmulatorDirectoryReader.cxx
  GaussianProcessEmulatorSingleFileReader.cxx
  GaussianProcessEmulatorSingleFileWriter.cxx
  Parameter.cxx
  Distribution.cxx
  ExternalModel.cxx
  GaussianDistribution.cxx
  LangevinSampler.cxx
  LatinHypercubeGenerator.cxx
  Model.cxx
  Sampler.cxx
  Paths.cxx
  ProcessPipe.c
  RegularStepGradientAscentSampler.cxx
  RuntimeParameterFileReader.cxx
  Sample.cxx
  MetropolisHastingsSampler.cxx
  Trace.cxx
  UniformDistribution.cxx
  PercentileGridSampler.cxx
)

add_library( DistributionSampling ${SRC_FILES} )
target_link_libraries( DistributionSampling madaisys )
set_target_properties( DistributionSampling
  PROPERTIES VERSION ${DistributionSamplingLibrary_VERSION_STRING}
)
