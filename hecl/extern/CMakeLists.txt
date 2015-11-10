#disable libBoo for FreeBSD for the time being
if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
add_subdirectory(libBoo)
set(BOO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libBoo/include)
endif()

add_subdirectory(libSquish)
add_subdirectory(xxhash)
if (NOT TARGET LogVisor)
add_subdirectory(LogVisor)
endif()
add_subdirectory(Athena)
add_subdirectory(libpng)
