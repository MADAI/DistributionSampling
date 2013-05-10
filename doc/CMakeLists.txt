# If we have reached this point, doxygen has been found on the system
add_custom_target( Documentation ALL
  COMMAND ${DOXYGEN_EXECUTABLE} ${DistributionSampling_SOURCE_DIR}/doc/doxyfile
  SOURCES ${DistributionSampling_SOURCE_DIR}/doc/doxyfile
  WORKING_DIRECTORY ${DistributionSampling_SOURCE_DIR}/doc
)

install( DIRECTORY ${DistributionSampling_SOURCE_DIR}/doc/html
  DESTINATION doc/madai/
)
