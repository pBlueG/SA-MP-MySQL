#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "log-core" for configuration "Release"
set_property(TARGET log-core APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(log-core PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/log-core2.so"
  IMPORTED_SONAME_RELEASE "log-core2.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS log-core )
list(APPEND _IMPORT_CHECK_FILES_FOR_log-core "${_IMPORT_PREFIX}/lib/log-core2.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
