include(FindPackageHandleStandardArgs)

find_path(SDK_INCLUDE_DIR NAMES plugin.h plugincommon.h
          HINTS ${SAMP_SDK_DIR}
                ENV SAMP_SDK_DIR
                ${SDK_DIR} 
                ENV SDK_DIR
          PATH_SUFFIXES sdk SDK
          DOC "Path to SA-MP plugin SDK"
          NO_SYSTEM_ENVIRONMENT_PATH
          NO_CMAKE_SYSTEM_PATH)

mark_as_advanced(SDK_INCLUDE_DIR)
find_package_handle_standard_args(SDK REQUIRED_VARS SDK_INCLUDE_DIR)
