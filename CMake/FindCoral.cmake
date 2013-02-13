set( coral_FOUND 0 )

set( coral_DIR_MESSAGE "coral not found. Set the coral_DIR cmake cache entry to the directory containing coral-config.cmake. This is either the root of the build tree, or PREFIX/lib for an installation.")

set( coral_DIR "coral_DIR-NOTFOUND" CACHE PATH "Build directory for coral" )

if ( coral_DIR )
  if ( EXISTS "${coral_DIR}/coral-config.cmake" )
    set( coral_FOUND 1 )
    include( "${coral_DIR}/coral-config.cmake" )
  endif()
endif()

set( LIB_PATHS
  /usr/lib
  /usr/local/lib
  /opt/local/lib
  /Users/quammen/tmp/lib  
)

set( INCLUDE_PATHS
  /usr/include
  /usr/local/include
  /opt/local/include
  /Users/quammen/tmp/include
  )

if ( NOT coral_DIR )
  find_library( coral_LIBRARY coral
    PATHS ${LIB_PATHS}
    )
  get_filename_component( coral_LIBRARY_DIR ${coral_LIBRARY} PATH )

  find_library( coralutils_LIBRARY coralutils
    PATHS ${LIB_PATHS}
    )
  get_filename_component( coralutils_LIBRARY_DIR ${coralutils_LIBRARY} PATH )

  find_library( xgraph_LIBRARY xgraph
    PATHS ${LIB_PATHS}
    )
  get_filename_component( xgraph_LIBRARY_DIR ${xgraph_LIBRARY} PATH )

  find_path( coral_INCLUDE_DIR coral/coral.h
    PATHS ${INCLUDE_PATHS}
    )
  find_path( coralutils_INCLUDE_DIR coralutils/coralutils.h
    PATHS ${INCLUDE_PATHS}
    )
  find_path( xgraph_INCLUDE_DIR xgraph/xgraph.h
    PATHS ${INCLUDE_PATHS}
    )

  # Need GSL
  find_package( GSL REQUIRED )

  if ( coral_LIBRARY AND coralutils_LIBRARY AND xgraph_LIBRARY AND coral_INCLUDE_DIRS )
    set( coral_FOUND 1 )

    set( coral_INCLUDE_DIRS
      "${coral_INCLUDE_DIR}/coral"
      "${coralutils_INCLUDE_DIR}/coralutils"
      "${xgraph_INCLUDE_DIR}/xgraph"
      ${GSL_INCLUDE_DIRS}
      )
    list( REMOVE_DUPLICATES coral_INCLUDE_DIRS )

    set( coral_LIBRARY_DIRS
      ${coral_LIBRARY_DIR}
      ${coralutils_LIBRARY_DIR}
      ${xgraph_LIBRARY_DIR}
      ${GSL_LIBRARY_DIRS}
      )
    list( REMOVE_DUPLICATES coral_LIBRARY_DIRS )

    set( coral_LIBRARIES
      ${coral_LIBRARY}
      ${coralutils_LIBRARY}
      ${xgraph_LIBRARY}
      ${GSL_LIBRARIES}
      )
    list( REMOVE_DUPLICATES coral_LIBRARIES )

  endif()

endif()
  
if ( NOT coral_FOUND )
  if ( coral_FIND_REQUIRED )
    message( FATAL_ERROR ${coral_DIR_MESSAGE})
  else( coral_FIND_REQUIRED )
    if ( NOT coral_FIND_QUIETLY )
      message( STATUS ${coral_DIR_MESSAGE})
    endif( NOT coral_FIND_REQUIRED )
  endif( coral_FIND_REQUIRED)
endif( NOT coral_FOUND )
