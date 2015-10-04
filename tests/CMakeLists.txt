include(AddSAMPPluginTest)

find_package(PawnCC REQUIRED)
find_package(SAMPServer REQUIRED)
find_package(SAMPServerCLI REQUIRED)

set(_MYSQL_PLUGIN_TEST_NAME "unit_test")


## compile script
list(APPEND _compile_flags
	${CMAKE_CURRENT_SOURCE_DIR}/${_MYSQL_PLUGIN_TEST_NAME}.pwn
	"-\;+"
	"-(+"
	-i${SAMPServer_INCLUDE_DIR}
	-i${CMAKE_CURRENT_SOURCE_DIR}/include
	-o${CMAKE_CURRENT_BINARY_DIR}/${_MYSQL_PLUGIN_TEST_NAME}
)

if(UNIX)
	string(REPLACE "\;" "\\$<SEMICOLON>" _compile_flags "${_compile_flags}")
	string(REPLACE "(" "\\(" _compile_flags "${_compile_flags}")
endif()

add_custom_command(
	OUTPUT            ${CMAKE_CURRENT_BINARY_DIR}/${_MYSQL_PLUGIN_TEST_NAME}.amx
	COMMAND           ${PawnCC_EXECUTABLE} ${_compile_flags}
	COMMENT           "Compiling test ${_MYSQL_PLUGIN_TEST_NAME}"
	DEPENDS           ${_MYSQL_PLUGIN_TEST_NAME}.pwn
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)


## copy test data
file(GLOB
	_MYSQL_PLUGIN_TEST_DATA 
	"${CMAKE_CURRENT_SOURCE_DIR}/test_data/*"
)
#TODO: scriptfiles test data has also to be copied
file(GLOB
	_MYSQL_PLUGIN_TEST_SCRIPTS
	"${CMAKE_CURRENT_SOURCE_DIR}/*.amx"
)
file(COPY ${_MYSQL_PLUGIN_TEST_DATA} DESTINATION "$ENV{SAMP_SERVER_ROOT}")
file(COPY ${_MYSQL_PLUGIN_TEST_SCRIPTS} DESTINATION "$ENV{SAMP_SERVER_ROOT}/gamemodes")


## execute SA-MP server
if(WIN32)
	set(ENV{PATH} "${SAMPServerCLI_DIR};$ENV{PATH}")
	string(REPLACE ";" "\\$<SEMICOLON>" ENV{PATH} "$ENV{PATH}")
else()
	set(ENV{PATH} "${SAMPServerCLI_DIR}:$ENV{PATH}")
endif()

add_samp_plugin_test(mysql-test
	TARGET mysql
	SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/${_MYSQL_PLUGIN_TEST_NAME}
	TIMEOUT 60
)