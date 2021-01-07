if(NOT WIN32 AND NOT APPLE AND NOT NX) # remove WIN32 when specter/freetype is gone
find_library(PNG_LIB NAMES png libpng)
endif()
if(NOT PNG_LIB)
message(STATUS "Using HECL's built-in libpng")
if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm(64)?")
set(INTRINSICS
        arm/arm_init.c
        arm/filter_neon.S
        arm/filter_neon_intrinsics.c
        arm/palette_neon_intrinsics.c)
elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86_64)
set(INTRINSICS
        intel/filter_sse2_intrinsics.c
        intel/intel_init.c)
endif()
add_library(png
    png.h
    pngconf.h
    pngdebug.h
    pnginfo.h
    pngpriv.h
    pngstruct.h
    pnglibconf.h

    png.c
    pngerror.c
    pngget.c
    pngmem.c
    pngpread.c
    pngread.c
    pngrio.c
    pngrtran.c
    pngrutil.c
    pngset.c
    pngtrans.c
    pngwio.c
    pngwrite.c
    pngwtran.c
    pngwutil.c
    ${INTRINSICS})
if(APPLE)
    target_compile_options(png PRIVATE -Wno-implicit-fallthrough)
endif()
target_link_libraries(png PUBLIC ${ZLIB_LIBRARIES})
target_include_directories(png PUBLIC ${ZLIB_INCLUDE_DIR})
set(PNG_LIBRARIES png CACHE PATH "PNG libraries" FORCE)
set(PNG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "PNG include path" FORCE)
else()
set(PNG_LIBRARIES ${PNG_LIB} CACHE PATH "PNG libraries" FORCE)
find_path(PNG_INCLUDE_DIR png.h)
set(PNG_INCLUDE_DIR ${PNG_INCLUDE_DIR} CACHE PATH "PNG include path" FORCE)
endif()
