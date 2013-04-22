# Add utility library
add_library( ApplicationUtilities
  ApplicationUtilities.cxx
)

add_executable(
  PCADecompose
  PCADecompose.cxx )
target_link_libraries( PCADecompose
  DistributionSampling
  ApplicationUtilities
)

add_executable(
  emulate
  emulate.cxx )
target_link_libraries( emulate
  DistributionSampling
  ApplicationUtilities
)

add_executable(
  trainEmulator
  trainEmulator.cxx)
target_link_libraries( trainEmulator
  DistributionSampling
  ApplicationUtilities
)

add_executable(
  basicTrain
  basicTrain.cxx)
target_link_libraries( basicTrain
  DistributionSampling
  ApplicationUtilities
)

add_executable(
  printThetas
  printThetas.cxx)
target_link_libraries( printThetas
  DistributionSampling
  ApplicationUtilities
)

add_executable(
  generateMCMCtrace
  generateMCMCtrace.cxx)
target_link_libraries( generateMCMCtrace
  DistributionSampling
  ApplicationUtilities
)

add_executable(
  generateMCMCtraceExternal
  generateMCMCtraceExternal.cxx)
target_link_libraries( generateMCMCtraceExternal
  DistributionSampling
  ApplicationUtilities
)

add_executable(
  generateTrainingPoints
  generateTrainingPoints.cxx)
target_link_libraries( generateTrainingPoints
  DistributionSampling
  ApplicationUtilities
)

add_executable(
  generatePercentileGridTrace
  generatePercentileGridTrace.cxx)
target_link_libraries( generatePercentileGridTrace
  DistributionSampling
  ApplicationUtilities
)

add_executable(
  generatePercentileGridTraceExternal
  generatePercentileGridTraceExternal.cxx)
target_link_libraries( generatePercentileGridTraceExternal
  DistributionSampling
  ApplicationUtilities
)
