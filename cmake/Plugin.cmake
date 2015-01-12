function(add_plugin name)
  add_library(${name} MODULE ${ARGN})

  set_target_properties(${name} PROPERTIES PREFIX "")

  if(CMAKE_COMPILER_IS_GNUCC OR ${CMAKE_CXX_COMPILER_ID} EQUAL Clang)
    set_property(TARGET ${name} APPEND_STRING PROPERTY COMPILE_FLAGS " -m32")
    set_property(TARGET ${name} APPEND_STRING PROPERTY LINK_FLAGS    " -m32")
  endif()

  if(CMAKE_COMPILER_IS_GNUCXX)
    set_property(TARGET ${name} APPEND_STRING PROPERTY
                 COMPILE_FLAGS " -Wno-attributes")
  endif()

  if(${CMAKE_CXX_COMPILER_ID} EQUAL Clang)
    set_property(TARGET ${name} APPEND_STRING PROPERTY
                 COMPILE_FLAGS " -Wno-ignored-attributes")
  endif()

  if(WIN32 AND CMAKE_COMPILER_IS_GNUCC)
    set_property(TARGET ${name} APPEND_STRING PROPERTY
                 LINK_FLAGS " -Wl,--enable-stdcall-fixup")
  endif()

  if(UNIX AND NOT WIN32)
    set_property(TARGET ${name} APPEND PROPERTY COMPILE_DEFINITIONS "LINUX")
  endif()
endfunction()
