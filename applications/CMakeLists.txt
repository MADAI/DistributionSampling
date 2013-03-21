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

add_executable(
  generateMCMCtrace
  generateMCMCtrace.cxx)
target_link_libraries(
  generateMCMCtrace
  DistributionSampling)

add_executable(
  generateMCMCtraceExternal
  generateMCMCtraceExternal.cxx)
target_link_libraries(
  generateMCMCtraceExternal
  DistributionSampling)
