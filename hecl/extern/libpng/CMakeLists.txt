if(NOT WIN32 AND NOT APPLE)
find_library(PNG_LIB png)
endif()

#if(WIN32 OR APPLE OR PNG_LIB STREQUAL PNG_LIB-NOTFOUND)
message(STATUS "Using HECL's built-in libpng")
include_directories(${ZLIB_INCLUDE_DIR})
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
    pngwutil.c)

set(PNG_LIB png CACHE PATH "PNG library" FORCE)
set(LIBPNG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "PNG include path" FORCE)

#else()
#find_path(LIBPNG_INCLUDE_DIR png.h)
#endif()
