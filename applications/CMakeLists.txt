# Add utility library
add_library( ApplicationUtilities
  ApplicationUtilities.cxx
  Defaults.cxx
)

target_link_libraries( ApplicationUtilities
  DistributionSampling
)

set( APPLICATIONS
  madai_pca_decompose
  madai_emulate
  madai_generate_trace
  madai_generate_training_points
  madai_train_emulator
  madai_print_default_settings
  madai_analyze_trace
  madai_generate_posterior_samples
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

if ( BUILD_TESTING )
  add_subdirectory( test )
endif()

# Install scripts
install( PROGRAMS
  madai_gnuplot_scatterplot_matrix
  madai_subsample_text_file
  madai_set_variable
  madai_catenate_traces
  madai_launch_multiple_madai_generate_trace
  madai_generate_posterior_samples
  DESTINATION bin
)
