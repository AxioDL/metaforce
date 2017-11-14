cmake_minimum_required(VERSION 3.10 FATAL_ERROR) # because of c++17
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


target_link_libraries(assetnameparser tinyxml2_static logvisor ${AN_PARSER_LIBS})
