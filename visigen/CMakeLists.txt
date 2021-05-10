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
  target_sources(visigen PRIVATE main.cpp metal/VISIRendererMetal.mm metal/VISIRendererMetal.hh)
  set_source_files_properties(metal/VISIRendererMetal.mm PROPERTIES COMPILE_FLAGS -fobjc-arc)
  find_library(METAL_LIBRARY Metal REQUIRED)
  target_link_libraries(visigen PRIVATE ${METAL_LIBRARY})
elseif(WIN32)
  target_sources(visigen PRIVATE
      MainWin.cpp
          opengl/VISIRendererOpenGL.cpp
          opengl/VISIRendererOpenGL.hpp
          vulkan/VISIRendererVulkan.cpp
          vulkan/VISIRendererVulkan.hpp
          vulkan/utils.cpp)
else ()
  bintoc(vk_fs.cpp vulkan/shader.frag.spv VK_FRAGMENT_SPV)
  bintoc(vk_vs.cpp vulkan/shader.vert.spv VK_VERTEX_SPV)
  target_sources(visigen PRIVATE
          main.cpp
          opengl/VISIRendererOpenGL.cpp
          opengl/VISIRendererOpenGL.hpp
          vulkan/VISIRendererVulkan.cpp
          vulkan/VISIRendererVulkan.hpp
          vulkan/utils.cpp
          vk_fs.cpp
          vk_vs.cpp)
endif ()

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
