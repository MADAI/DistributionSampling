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
  printDefaultSettings
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

# Install binary executables
install( TARGETS ${APPLICATIONS}
  RUNTIME DESTINATION bin
)

# Install scripts
install( TARGETS madai_subsample_text_file
  RUNTIME DESTINATION bin
)
