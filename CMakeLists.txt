cmake_minimum_required(VERSION 3.0)
project(RetroCommon)
if (NOT TARGET NOD)
    add_subdirectory(NODLib)
endif()
include_directories(${ATHENA_INCLUDE_DIR} ${LOG_VISOR_INCLUDE_DIR} ${ANGELSCRIPT_INCLUDE_DIR})
set(NOD_LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/NODLib/include)
add_subdirectory(DataSpec)
add_subdirectory(Runtime)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
include_directories(include)
add_library(RetroCommon
            src/RetroCommon.cpp include/RetroCommon.hpp
            src/MREADecompress.cpp)
