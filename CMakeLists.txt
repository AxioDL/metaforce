cmake_minimum_required(VERSION 3.0)
project(PathShagged)
if(MSVC)
  # Shaddup MSVC
  add_definitions(-DUNICODE=1 -D_UNICODE=1 -D__SSE__=1 -D_CRT_SECURE_NO_WARNINGS=1 -DD_SCL_SECURE_NO_WARNINGS=1 /wd4267 /wd4244 /wd4305)
else()
  if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
    message(WARNING "GCC needs -fpermissive for nested type redeclarations; expect lotsa warnings!!")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wno-multichar -fno-exceptions")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
include_directories(/usr/local/include)
endif()

set(BOO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/hecl/extern/libBoo/include)
set(HECL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/hecl/include
                     ${CMAKE_CURRENT_SOURCE_DIR}/hecl/blender)

set(HECL_DATASPEC_DECLS
"/* RetroCommon specs */
namespace Retro
{
    extern HECL::Database::DataSpecEntry SpecEntMP1;
    extern HECL::Database::DataSpecEntry SpecEntMP2;
    extern HECL::Database::DataSpecEntry SpecEntMP3;
}")
set(HECL_DATASPEC_PUSHES
"    /* RetroCommon */
    HECL::Database::DATA_SPEC_REGISTRY.push_back(&Retro::SpecEntMP1);
    HECL::Database::DATA_SPEC_REGISTRY.push_back(&Retro::SpecEntMP2);
    HECL::Database::DATA_SPEC_REGISTRY.push_back(&Retro::SpecEntMP3);")
add_subdirectory(hecl)
add_subdirectory(libSpecter)
set(SPECTER_INCLUDE_DIR libSpecter/include libSpecter/freetype2/include)
add_subdirectory(NODLib)
set(NODLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/NODLib/include)
add_definitions(-DZE_ATHENA_TYPES=1)
set(MATHLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libSpecter/MathLib/include)
include_directories(${ATHENA_INCLUDE_DIR} ${LOG_VISOR_INCLUDE_DIR} ${HECL_INCLUDE_DIR}
                    ${NODLIB_INCLUDE_DIR} ${MATHLIB_INCLUDE_DIR} ${BOO_INCLUDE_DIR}
                    ${SPECTER_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(DataSpec)
#disable Runtime on FreeBSD for now
if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
add_subdirectory(Editor)
add_subdirectory(Runtime)
endif()

find_package(Git)
if(GIT_FOUND)
  message("git found: ${GIT_EXECUTABLE}")
    # Get the current working branch
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE GIT_BRANCH OUTPUT_STRIP_TRAILING_WHITESPACE )

    # Get the latest abbreviated commit hash of the working branch
    execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%h WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE GIT_COMMIT_HASH OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE GIT_COMMIT_HASH_FULL OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%ad WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE GIT_COMMIT_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)

    configure_file(${CMAKE_SOURCE_DIR}/version.h.in ${CMAKE_SOURCE_DIR}/version.h)
endif()
