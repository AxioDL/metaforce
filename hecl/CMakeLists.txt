if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
cmake_minimum_required(VERSION 3.0)
project(hecl)
if(MSVC)
  # Shaddup MSVC
  add_definitions(-DUNICODE=1 -D_UNICODE=1 -D_CRT_SECURE_NO_WARNINGS=1 /wd4267 /wd4244)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wno-multichar -fno-exceptions")
endif()
endif()

configure_file(DataSpecRegistry.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/DataSpecRegistry.hpp @ONLY)

set(ATHENA_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/athena/include)
set(ATHENA_INCLUDE_DIR ${ATHENA_INCLUDE_DIR} PARENT_SCOPE)
set(SQUISH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/libSquish)
set(SQUISH_INCLUDE_DIR ${SQUISH_INCLUDE_DIR} PARENT_SCOPE)

set(BOO_INCLUDE_DIR extern/boo/include)

add_subdirectory(bintoc)
add_subdirectory(extern)
add_definitions(${BOO_SYS_DEFINES})
include_directories(include blender ${LOGVISOR_INCLUDE_DIR} ${ATHENA_INCLUDE_DIR} ${BOO_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
add_subdirectory(lib)
add_subdirectory(blender)
add_subdirectory(driver)
install(DIRECTORY include/hecl DESTINATION include/hecl)

# Runtime test
add_subdirectory(test)
