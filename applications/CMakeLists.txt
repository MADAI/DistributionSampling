# Add utility library
add_library( ApplicationUtilities
  ApplicationUtilities.cxx
  Defaults.cxx
)

set( APPLICATIONS
  PCADecompose
  basicTrain
  emulate
  generateMCMCtrace
  generateMCMCtraceExternal
  generatePercentileGridTrace
  generatePercentileGridTraceExternal
  generateTrainingPoints
  trainEmulator
)

foreach( application ${APPLICATIONS} )
  add_executable(
    ${application}
    ${application}.cxx
    )
  target_link_libraries( ${application}
    DistributionSampling
    ApplicationUtilities
    )
endforeach()


install( TARGETS ${APPLICATIONS}
  RUNTIME DESTINATION bin
)
