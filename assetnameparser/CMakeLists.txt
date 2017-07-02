cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR) # because of CMAKE_CXX_STANDARD
add_subdirectory(tinyxml2)
add_executable(assetnameparser "main.cpp")
include_directories(${LOGVISOR_INCLUDE_DIR})

set(AN_PARSER_LIBS "")
if (UNIX)
    list(APPEND AN_PARSER_LIBS pthread)
    if(UNIX AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        list(APPEND AN_PARSER_LIBS dl)
    endif()
endif()


target_link_libraries(assetnameparser tinyxml2 logvisor ${AN_PARSER_LIBS})
