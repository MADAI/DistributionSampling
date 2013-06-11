set( MADAIEmulator_FOUND 0 )

set( MADAIEmulator_DIR_MESSAGE "MADAIEmulator not found. Set the MADAIEmulator_DIR cmake cache entry to the directory containing MADAIEmulatorConfig.cmake. This is either the root of the build tree, or PREFIX/lib for an installation.")

set( MADAIEmulator_DIR "MADAIEmulator_DIR-NOTFOUND" CACHE PATH "Build directory for MADAIEmulator" )

if ( MADAIEmulator_DIR )
  if ( EXISTS "${MADAIEmulator_DIR}/MADAIEmulatorConfig.cmake" )
    set( MADAIEmulator_FOUND 1 )
    include( "${MADAIEmulator_DIR}/MADAIEmulatorConfig.cmake" )
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

if ( NOT MADAIEmulator_DIR )
  find_library( Emu_LIBRARY Emu
    PATHS ${LIB_PATHS}
    )
  get_filename_component( Emu_LIBRARY_DIR ${Emu_LIBRARY} PATH )

  find_library( EmuPlusPlus_LIBRARY EmuPlusPlus
    PATHS ${LIB_PATHS}
    )
  get_filename_component( EmuPlusPlus_LIBRARY_DIR ${EmuPlusPlus_LIBRARY} PATH )

  find_library( rbind_LIBRARY RBIND
    PATHS ${LIB_PATHS}
    )
  get_filename_component( rbind_LIBRARY_DIR ${rbind_LIBRARY} PATH )

  find_path( Emu_INCLUDE_DIR libEmu/emulator.h
    PATHS ${INCLUDE_PATHS}
    )
  find_path( EmuPlusPlus_INCLUDE_DIR EmuPlusPlus/EmuPlusPlus.h
    PATHS ${INCLUDE_PATHS}
    )
  find_path( rbind_INCLUDE_DIR libRbind/rbind.h
    PATHS ${INCLUDE_PATHS}
    )

  # Need GSL
  find_package( GSL REQUIRED )

  if ( Emu_LIBRARY AND EmuPlusPlus_LIBRARY AND rbind_LIBRARY AND Emu_INCLUDE_DIR )
    set( MADAIEmulator_FOUND 1 )

    set( MADAIEmulator_INCLUDE_DIRS
      "${Emu_INCLUDE_DIR}"
      "${EmuPlusPlus_INCLUDE_DIR}"
      "${rbind_INCLUDE_DIR}"
      ${GSL_INCLUDE_DIRS}
      )
    list( REMOVE_DUPLICATES MADAIEmulator_INCLUDE_DIRS )

    set( MADAIEmulator_LIBRARY_DIRS
      ${Emu_LIBRARY_DIR}
      ${EmuPlusPlus_LIBRARY_DIR}
      ${rbind_LIBRARY_DIR}
      ${GSL_LIBRARY_DIRS}
      )
    list( REMOVE_DUPLICATES MADAIEmulator_LIBRARY_DIRS )

    set( MADAIEmulator_LIBRARIES
      ${Emu_LIBRARY}
      ${EmuPlusPlus_LIBRARY}
      ${rbind_LIBRARY}
      ${GSL_LIBRARIES}
      )
    list( REMOVE_DUPLICATES MADAIEmulator_LIBRARIES )

  endif()

endif()

if ( NOT MADAIEmulator_FOUND )
  if ( MADAIEmulator_FIND_REQUIRED )
    message( FATAL_ERROR ${MADAIEmulator_DIR_MESSAGE})
  else( MADAIEmulator_FIND_REQUIRED )
    if ( NOT MADAIEmulator_FIND_QUIETLY )
      message( STATUS ${MADAIEmulator_DIR_MESSAGE})
    endif( NOT MADAIEmulator_FIND_REQUIRED )
  endif( MADAIEmulator_FIND_REQUIRED)
endif( NOT MADAIEmulator_FOUND )
