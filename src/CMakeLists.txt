set( SRC_FILES
  Random.cxx
  GaussianProcessEmulator.cxx
  GaussianProcessEmulatedModel.cxx
  GaussianProcessEmulatorDirectoryFormatIO.cxx
  Parameter.cxx
  Distribution.cxx
  ExternalModel.cxx
  GaussianDistribution.cxx
  LangevinSampler.cxx
  LatinHypercubeGenerator.cxx
  Model.cxx
  Sampler.cxx
  Paths.cxx
  ProcessPipe.cxx
  RegularStepGradientAscentSampler.cxx
  RuntimeParameterFileReader.cxx
  Sample.cxx
  MetropolisHastingsSampler.cxx
  UniformDistribution.cxx
  PercentileGridSampler.cxx
  SamplerCSVWriter.cxx
  System.cxx
)

add_library( DistributionSampling ${SRC_FILES} )
target_link_libraries( DistributionSampling madaisys )
set_target_properties( DistributionSampling
  PROPERTIES VERSION ${DistributionSamplingLibrary_VERSION_STRING}
)

install( TARGETS DistributionSampling
  LIBRARY DESTINATION lib/madai
  ARCHIVE DESTINATION lib/madai
)

install( DIRECTORY .
  DESTINATION include/madai/DistributionSampling
  FILES_MATCHING PATTERN *.h
)
