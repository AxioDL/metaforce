cmake_minimum_required(VERSION 3.0)
project(PathShagged)
if(MSVC)
  # Shaddup MSVC
  add_definitions(-DUNICODE=1 -D_UNICODE=1 -D_CRT_SECURE_NO_WARNINGS=1 -DD_SCL_SECURE_NO_WARNINGS=1 /wd4267 /wd4244 /wd4305)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wno-multichar -fno-exceptions")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
include_directories(/usr/local/include)
endif()

#disable libBoo for FreeBSD for the time being
if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
add_subdirectory(libBoo)
set(BOO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libBoo/include)
endif()

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
add_subdirectory(NODLib)
set(NODLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/NODLib/include)
add_subdirectory(MathLib)
set(MATHLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/MathLib/include)
add_definitions(-DZE_ATHENA_TYPES=1)
include_directories(${ATHENA_INCLUDE_DIR} ${LOG_VISOR_INCLUDE_DIR} ${HECL_INCLUDE_DIR}
                    ${NODLIB_INCLUDE_DIR} ${MATHLIB_INCLUDE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(DataSpec)
#disable Runtime on FreeBSD for now
if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
add_subdirectory(Runtime)
endif()
