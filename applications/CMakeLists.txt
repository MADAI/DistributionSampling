# Add utility library
add_library( ApplicationUtilities
  ApplicationUtilities.cxx
  Defaults.cxx
)

set( APPLICATIONS
  madai_pca_decompose
  madai_basic_train
  madai_emulate
  madai_generate_mcmc_trace
  madai_generate_mcmc_trace_external
  madai_generate_percentile_grid_trace
  madai_generate_percentile_grid_trace_external
  madai_generate_training_points
  madai_train_emulator
  madai_print_default_settings
  madai_mcmc_analyze_trace
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
install( PROGRAMS
  madai_subsample_text_file
  madai_set_variable
  DESTINATION bin
)
