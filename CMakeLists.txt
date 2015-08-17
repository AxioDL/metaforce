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

set(HECL_SUBPROJECT TRUE)
add_subdirectory(hecl)
add_subdirectory(NODLib)
include_directories(${ATHENA_INCLUDE_DIR} ${LOG_VISOR_INCLUDE_DIR} ${HECL_INCLUDE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR})
set(NOD_LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/NODLib/include)
add_subdirectory(DataSpec)
add_subdirectory(Runtime)
