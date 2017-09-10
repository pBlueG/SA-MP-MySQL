cmake_minimum_required(VERSION 3.1)
project(mysql)

set(MYSQL_PLUGIN_VERSION "R41-4")

option (BUILD_TESTS "Build tests" OFF)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/libs/cmake")

if(UNIX)
	#force 32bit compilation and hide non-exported symbols
	set(CMAKE_CXX_FLAGS "-m32 -fvisibility=hidden")
	set(CMAKE_C_FLAGS "-m32 -fvisibility=hidden")
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)
endif()


find_package(MySQLCAPI 5.5 REQUIRED)


set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.57 QUIET)

if(NOT Boost_FOUND)
	set(BOOST_ROOT "${BOOST_ROOT}" CACHE PATH "Set the Boost root folder.")
	mark_as_advanced(CLEAR BOOST_ROOT)
	message(FATAL_ERROR "Could NOT find Boost; specify additional path.")
endif()
mark_as_advanced(BOOST_ROOT Boost_DIR)

include_directories("${Boost_INCLUDE_DIR}" "${MYSQLCAPI_INCLUDE_DIR}")


option(LOGCORE_INSTALL_DEV OFF)
mark_as_advanced(LOGCORE_INSTALL_DEV)
add_subdirectory(libs/samp-log-core)


add_subdirectory(src)


