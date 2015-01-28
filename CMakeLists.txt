cmake_minimum_required(VERSION 2.8)
project(mysql)

set(MYSQL_PLUGIN_VERSION "R40")

option (BUILD_TESTS "Build tests" OFF)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/libs/cmake")

if(UNIX)
	#force C++11 flag and 32bit compilation
	set(CMAKE_CXX_FLAGS "-m32 -std=c++11")
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)
endif()

#compiler with C++11 support required
include(CheckCXXSourceCompiles)
check_cxx_source_compiles(
	"
	#include <tuple>
	#include <functional>
	#include <unordered_map>
	#include <forward_list>
	#include <string>

	class CTest
	{
	public:
		CTest() = default;
		~CTest() = default;
		CTest(const CTest &rhs) = delete;
		CTest(CTest &&mv)
		{
			m_Num = mv.m_Num;
			m_NumPtr = mv.m_NumPtr;
		}
		
		int m_Num = 42;
		int *m_NumPtr = nullptr;
		std::unordered_map<std::string, int> m_Map;
		std::forward_list<int> m_ForwardList;
	};

	int main(void)
	{
		int RefVar = 123;
		std::tuple<
			int, 
			std::function<bool(int, int&)>> 
		MyTuple(
			42, [&RefVar](int num1, int &sub_ref)
			{
				RefVar += num1;
				sub_ref += RefVar;
				return true;
			}
		);
		
		enum class StronglyTypedEnum
		{
			ENUM_VAR1,
			ENUM_VAR2
		};
		
		CTest test_class;
		
		return 0;
	}
	" 
	COMPILER_SUPPORTS_CXX11)

if(NOT COMPILER_SUPPORTS_CXX11)
	message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} does not fulfill all required standards. Please use a different C++ compiler.")
endif()


find_package(MySQLCAPI 5.5.40 REQUIRED)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.57 QUIET 
	COMPONENTS atomic chrono date_time filesystem system thread)

if(NOT Boost_FOUND)
	set(BOOST_ROOT "${BOOST_ROOT}" CACHE PATH "Set the Boost root folder.")
	set(BOOST_LIBRARYDIR "${BOOST_LIBRARYDIR}" CACHE PATH "Set the Boost library folder.")
	mark_as_advanced(CLEAR BOOST_ROOT BOOST_LIBRARYDIR)
	message(FATAL_ERROR "Could NOT find Boost; specify additional paths.")
endif()
mark_as_advanced(BOOST_ROOT BOOST_LIBRARYDIR Boost_DIR)


include_directories("${Boost_INCLUDE_DIR}" "${MYSQLCAPI_INCLUDE_DIR}")
if(MSVC)
	#needed for Boost's auto-link feature
	link_directories("${Boost_LIBRARY_DIRS}")
endif()


add_subdirectory(src)
