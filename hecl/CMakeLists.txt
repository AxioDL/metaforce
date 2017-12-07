if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
cmake_minimum_required(VERSION 3.10 FATAL_ERROR) # because of c++17
project(hecl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
  # Shaddup MSVC
  add_definitions(-DUNICODE=1 -D_UNICODE=1 -D_CRT_SECURE_NO_WARNINGS=1 /wd4267 /wd4244)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-multichar -fno-exceptions")
endif()
endif()

configure_file(DataSpecRegistry.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/DataSpecRegistry.hpp @ONLY)

set(ATHENA_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/athena/include)
set(ATHENA_INCLUDE_DIR ${ATHENA_INCLUDE_DIR} PARENT_SCOPE)
set(SQUISH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/libSquish)
set(SQUISH_INCLUDE_DIR ${SQUISH_INCLUDE_DIR} PARENT_SCOPE)

set(BOO_INCLUDE_DIR extern/boo/include)

add_subdirectory(bintoc)

if(NOT TARGET bintoc)
  # Use native if cross compiling
  find_package(hecl-bintoc REQUIRED)
endif()

add_subdirectory(extern)

if(NOT TARGET atdna)
  # Import native atdna if cross-compiling
  find_package(atdna REQUIRED)
  if(NOT TARGET atdna)
    message(FATAL_ERROR "atdna required for building hecl; please verify LLVM installation")
  endif()
endif()

add_definitions(${BOO_SYS_DEFINES})
include_directories(include blender ${LOGVISOR_INCLUDE_DIR} ${ATHENA_INCLUDE_DIR}
                    ${BOO_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${BOO_SYS_INCLUDES})
add_subdirectory(lib)
add_subdirectory(blender)
add_subdirectory(driver)
install(DIRECTORY include/hecl DESTINATION include/hecl)

# Runtime test
add_subdirectory(test)
