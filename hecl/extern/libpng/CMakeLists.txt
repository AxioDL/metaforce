find_package(PNG)
if(NOT PNG_FOUND)
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

set(PNG_LIBRARIES png CACHE PATH "PNG library" FORCE)
set(PNG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "PNG include path" FORCE)

else()
find_path(PNG_INCLUDE_DIR png.h)
set(PNG_LIBRARIES ${PNG_LIBRARIES} CACHE PATH "PNG library" FORCE)
endif()