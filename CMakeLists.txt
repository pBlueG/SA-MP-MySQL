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


find_package(MySQLCAPI REQUIRED)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.82 QUIET)
if(NOT Boost_FOUND)
	set(BOOST_ROOT "${BOOST_ROOT}" CACHE PATH "Set the Boost root folder.")
	mark_as_advanced(CLEAR BOOST_ROOT)
	message(FATAL_ERROR "Could NOT find Boost; specify additional path.")
endif()
mark_as_advanced(BOOST_ROOT Boost_DIR)

add_subdirectory(libs/fmt)
mark_as_advanced(FMT_DOC FMT_INSTALL FMT_PEDANTIC FMT_TEST FMT_USE_CPP11)

find_package(log-core REQUIRED CONFIG)


add_subdirectory(src)