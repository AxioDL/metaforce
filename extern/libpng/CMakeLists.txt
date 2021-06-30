if (APPLE)
  if (NOT PNG_DIR)
    # hint at macports libpng (build with +universal)
    set(PNG_DIR /opt/local/lib)
  endif ()
  list(LENGTH CMAKE_OSX_ARCHITECTURES num_archs)
  if (num_archs GREATER 1)
    # disable default search paths so we don't use homebrew's non-universal libpng
    set(PNG_SEARCH NO_DEFAULT_PATHS NO_CMAKE_FIND_ROOT_PATH NO_CMAKE_SYSTEM_PATH)
  endif ()
  # only consider static libs
  find_library(PNG_LIB NAMES libpng.a HINTS ${PNG_DIR} ${PNG_SEARCH})
  if (PNG_LIB)
    find_path(PNG_INCLUDE_DIR png.h HINTS "${PNG_LIB}/../../include" NO_DEFAULT_PATHS NO_CMAKE_FIND_ROOT_PATH NO_CMAKE_SYSTEM_PATH)
    add_library(png STATIC IMPORTED GLOBAL)
    set_target_properties(png PROPERTIES IMPORTED_LOCATION ${PNG_LIB})
    target_include_directories(png INTERFACE ${PNG_INCLUDE_DIR})
    target_link_libraries(png INTERFACE ${ZLIB_LIBRARIES})
    set(PNG_LIBRARIES png CACHE PATH "PNG libraries" FORCE)
    message(STATUS "Using static libpng at ${PNG_LIB}, include: ${PNG_INCLUDE_DIR}")
    set(PNG_FOUND YES)
  endif ()
else ()
  find_package(PNG)
  if (PNG_FOUND)
    set_target_properties(PNG::PNG PROPERTIES IMPORTED_GLOBAL TRUE)
    set(PNG_LIBRARIES PNG::PNG CACHE STRING "PNG libraries" FORCE)
  endif ()
endif ()
if (NOT PNG_FOUND)
  message(STATUS "Using built-in libpng")
  if ("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm(64)?" OR (APPLE AND "arm64" IN_LIST CMAKE_OSX_ARCHITECTURES))
    list(APPEND INTRINSICS
        arm/arm_init.c
        arm/filter_neon.S
        arm/filter_neon_intrinsics.c
        arm/palette_neon_intrinsics.c)
  endif ()
  if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" OR (APPLE AND "x86_64" IN_LIST CMAKE_OSX_ARCHITECTURES))
    list(APPEND INTRINSICS
        intel/filter_sse2_intrinsics.c
        intel/intel_init.c)
  endif ()
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
  if (APPLE)
    target_compile_options(png PRIVATE -Wno-implicit-fallthrough)
  endif ()
  target_link_libraries(png PUBLIC ${ZLIB_LIBRARIES})
  target_include_directories(png INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
  set(PNG_LIBRARIES png CACHE PATH "PNG libraries" FORCE)
endif ()
