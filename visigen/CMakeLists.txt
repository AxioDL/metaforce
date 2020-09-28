if(NOT WINDOWS_STORE)
cmake_minimum_required(VERSION 3.10 FATAL_ERROR) # because of c++17
project(visigen)

if (NOT MSVC)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

add_executable(visigen
  VISIBuilder.cpp
  VISIBuilder.hpp
  VISIRenderer.cpp
  VISIRenderer.hpp
  glew.c
)

if(MSVC)
  target_compile_options(visigen PRIVATE /EHsc)
else()
  target_compile_options(visigen PRIVATE
    -fno-exceptions

    -Werror=return-type
    -Wno-multichar
    -Wno-narrowing
    -Wno-nullability-completeness
  )
endif()

if(APPLE)
  target_sources(visigen PRIVATE MainMac.mm)
  set_source_files_properties(MainMac.mm PROPERTIES COMPILE_FLAGS -fobjc-arc)
elseif(WIN32)
  target_sources(visigen PRIVATE MainWin.cpp)
else()
  target_sources(visigen PRIVATE MainXlib.cpp)
endif()

set_target_properties(visigen PROPERTIES COMPILE_DEFINITIONS URDE_DLPACKAGE="${URDE_DLPACKAGE}")

if(APPLE)
  find_library(OPENGL_LIBRARY OpenGL)
  target_link_libraries(visigen PRIVATE ${OPENGL_LIBRARY})
else()
  include(FindPkgConfig)
  pkg_check_modules(xstuff IMPORTED_TARGET x11 gl)
  target_link_libraries(visigen PRIVATE PkgConfig::xstuff)
  target_compile_options(visigen PRIVATE -pthread)
  target_link_options(visigen PRIVATE -pthread)
endif()

target_link_libraries(visigen PRIVATE
  athena-core
  hecl-light
  logvisor
  lzokay
  xxhash
  zeus
  ${ZLIB_LIBRARIES}
)
endif()
