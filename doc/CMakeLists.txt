# If we have reached this point, doxygen has been found on the system
configure_file( Doxyfile.in ${DistributionSampling_BINARY_DIR}/doc/Doxyfile @ONLY )

add_custom_target( Documentation
  COMMAND ${DOXYGEN_EXECUTABLE} ${DistributionSampling_BINARY_DIR}/doc/Doxyfile
  SOURCES ${DistributionSampling_SOURCE_DIR}/doc/Doxyfile.in
  WORKING_DIRECTORY ${DistributionSampling_SOURCE_DIR}/doc
)

install( DIRECTORY ${DistributionSampling_BINARY_DIR}/doc/html
  DESTINATION doc/madai/
)
