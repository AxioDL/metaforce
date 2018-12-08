project (QuaZip)
cmake_minimum_required(VERSION 2.8)

find_package(Qt5Core)
find_package(ZLIB)

include_directories(${Qt5Core_INCLUDE_DIRS})

# All build libraries are moved to this directory
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)")
set(LIB_DESTINATION "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE STRING "Library directory name" FORCE)

add_subdirectory(quazip)
