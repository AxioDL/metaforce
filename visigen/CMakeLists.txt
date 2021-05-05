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
  target_sources(visigen PRIVATE MainMac.mm VISIRendererMetal.mm VISIRendererMetal.hh)
  set_source_files_properties(MainMac.mm PROPERTIES COMPILE_FLAGS -fobjc-arc)
  set_source_files_properties(VISIRendererMetal.mm PROPERTIES COMPILE_FLAGS -fobjc-arc)
  find_library(METAL_LIBRARY Metal REQUIRED)
  target_link_libraries(visigen PRIVATE ${METAL_LIBRARY})
elseif(WIN32)
  target_sources(visigen PRIVATE
      MainWin.cpp
      VISIRendererOpenGL.cpp
      VISIRendererOpenGL.hpp)
else()
  target_sources(visigen PRIVATE
      MainXlib.cpp
      VISIRendererOpenGL.cpp
      VISIRendererOpenGL.hpp)
endif()

target_link_libraries(visigen PRIVATE
  athena-core
  glew
  hecl-light
  logvisor
  lzokay
  xxhash
  zeus
  png
  ${ZLIB_LIBRARIES}
)
target_include_directories(visigen PRIVATE ${PNG_INCLUDE_DIR})
endif()
