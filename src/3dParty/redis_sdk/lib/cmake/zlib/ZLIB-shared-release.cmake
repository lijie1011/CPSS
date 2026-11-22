#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ZLIB::ZLIB" for configuration "Release"
set_property(TARGET ZLIB::ZLIB APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(ZLIB::ZLIB PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/z.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/z.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS ZLIB::ZLIB )
list(APPEND _IMPORT_CHECK_FILES_FOR_ZLIB::ZLIB "${_IMPORT_PREFIX}/lib/z.lib" "${_IMPORT_PREFIX}/bin/z.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
