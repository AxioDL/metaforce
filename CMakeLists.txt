cmake_minimum_required(VERSION 3.0)
project(RetroCommon)

cmake_minimum_required(VERSION 3.0)
project(hecl)
if(MSVC)
  # Shaddup MSVC
  add_definitions(-DUNICODE=1 -D_UNICODE=1 -D_CRT_SECURE_NO_WARNINGS=1 /wd4267 /wd4244)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-multichar -fno-exceptions")
endif()

add_subdirectory(libBoo)

set(HECL_SUBPROJECT TRUE)
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
include_directories(${ATHENA_INCLUDE_DIR} ${LOG_VISOR_INCLUDE_DIR} ${HECL_INCLUDE_DIR}
                    ${NODLIB_INCLUDE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(DataSpec)
add_subdirectory(Runtime)
