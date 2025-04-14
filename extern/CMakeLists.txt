# Enable MoltenVK
#if (CMAKE_SYSTEM_NAME STREQUAL Darwin)
#  set(DAWN_ENABLE_VULKAN ON CACHE BOOL "Enable compilation of the Vulkan backend" FORCE)
#endif()
if (CMAKE_SYSTEM_NAME STREQUAL Linux)
  set(DAWN_USE_WAYLAND ON CACHE BOOL "Enable support for Wayland surface" FORCE)
endif ()
add_subdirectory(aurora)

if (NOT TARGET spdlog)
  set(SPDLOG_NO_EXCEPTIONS ON CACHE BOOL "Compile with -fno-exceptions. Call abort() on any spdlog exceptions" FORCE)
  set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "Use external fmt library instead of bundled" FORCE)
  add_subdirectory(spdlog EXCLUDE_FROM_ALL)
endif ()

if (WIN32 AND NOT TARGET nowide)
  include(FetchContent)
  FetchContent_Declare(
    nowide
    URL https://github.com/boostorg/nowide/releases/download/v11.3.0/nowide_standalone_v11.3.0.tar.gz
    URL_HASH SHA256=153ac93173c8de9c08e7701e471fa750f84c27e51fe329570c5aa06016591f8c
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    EXCLUDE_FROM_ALL
  )
  FetchContent_MakeAvailable(nowide)
endif ()

add_subdirectory(nod EXCLUDE_FROM_ALL)
add_subdirectory(jbus EXCLUDE_FROM_ALL)
add_subdirectory(kabufuda EXCLUDE_FROM_ALL)

option(OPTICK_ENABLED "Enable profiling with Optick" OFF)
set(OPTICK_USE_VULKAN ${DAWN_ENABLE_VULKAN} CACHE BOOL "Built-in support for Vulkan" FORCE)
set(OPTICK_INSTALL_TARGETS OFF CACHE BOOL "Should optick be installed? Set to OFF if you use add_subdirectory to include Optick." FORCE)
add_subdirectory(optick)
if (NOT MSVC)
  target_compile_options(OptickCore PRIVATE -Wno-implicit-fallthrough)
endif ()

add_subdirectory(libjpeg-turbo EXCLUDE_FROM_ALL)
add_subdirectory(zeus EXCLUDE_FROM_ALL)
add_subdirectory(musyx EXCLUDE_FROM_ALL)
