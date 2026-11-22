#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Redis::redis_protocol" for configuration "Release"
set_property(TARGET Redis::redis_protocol APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Redis::redis_protocol PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/redis_protocol.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/redis_protocol.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS Redis::redis_protocol )
list(APPEND _IMPORT_CHECK_FILES_FOR_Redis::redis_protocol "${_IMPORT_PREFIX}/lib/redis_protocol.lib" "${_IMPORT_PREFIX}/bin/redis_protocol.dll" )

# Import target "Redis::redis_middleware" for configuration "Release"
set_property(TARGET Redis::redis_middleware APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Redis::redis_middleware PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/redis_middleware.lib"
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "hiredis::hiredis;ZLIB::ZLIB"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/redis_middleware.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS Redis::redis_middleware )
list(APPEND _IMPORT_CHECK_FILES_FOR_Redis::redis_middleware "${_IMPORT_PREFIX}/lib/redis_middleware.lib" "${_IMPORT_PREFIX}/bin/redis_middleware.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
