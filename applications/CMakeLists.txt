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
install( FILES madai_subsample_text_file
  PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  DESTINATION bin
)
