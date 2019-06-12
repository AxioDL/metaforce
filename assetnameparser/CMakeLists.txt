cmake_minimum_required(VERSION 3.10 FATAL_ERROR) # because of c++17

set(TINYXML_LIB_VERSION "5.0.1")
set(TINYXML_LIB_SOVERSION "5")
add_library(tinyxml2_static STATIC tinyxml2/tinyxml2.cpp tinyxml2/tinyxml2.h)
set_target_properties(tinyxml2_static PROPERTIES
        COMPILE_DEFINITONS "TINYXML2_EXPORT"
        VERSION "${TINYXML_LIB_VERSION}"
        SOVERSION "${TINYXML_LIB_SOVERSION}")
set_target_properties(tinyxml2_static PROPERTIES OUTPUT_NAME tinyxml2)
if (NOT MSVC)
  target_compile_options(tinyxml2_static PRIVATE -Wno-implicit-fallthrough)
endif()

add_executable(assetnameparser "main.cpp")

set(AN_PARSER_LIBS "")
if (UNIX)
  list(APPEND AN_PARSER_LIBS pthread)
  if(UNIX AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    list(APPEND AN_PARSER_LIBS dl)
  endif()
endif()

target_link_libraries(assetnameparser tinyxml2_static logvisor ${AN_PARSER_LIBS})
