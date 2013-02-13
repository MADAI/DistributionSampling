set( LIBRARIES 	
  ModelOptimization
  ${CORELIBS}
)

add_executable(        mcmc
                       mcmc.cc )
target_link_libraries( mcmc ${LIBRARIES} )
