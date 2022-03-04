if (WIN32)
  set(SDL_LIBC ON CACHE BOOL "Use the system C library" FORCE)
endif ()
add_subdirectory(../extern/SDL SDL2 EXCLUDE_FROM_ALL)
if (NOT MSVC)
  target_compile_options(SDL2-static PRIVATE -Wno-implicit-fallthrough -Wno-shadow)
endif ()

add_subdirectory(../extern/dawn dawn EXCLUDE_FROM_ALL)
target_compile_definitions(dawn_native PRIVATE
    DAWN_ENABLE_VULKAN_VALIDATION_LAYERS
    DAWN_VK_DATA_DIR="vulkandata")
if (NOT MSVC)
  target_compile_options(SPIRV-Tools-static PRIVATE -Wno-implicit-fallthrough)
  target_compile_options(SPIRV-Tools-opt PRIVATE -Wno-implicit-fallthrough)
endif ()

add_library(aurora STATIC
    lib/aurora.cpp
    lib/gpu.cpp
    lib/imgui.cpp
    lib/input.cpp
    lib/dawn/BackendBinding.cpp
    lib/dawn/Hacks.cpp
    lib/gfx/common.cpp
    lib/gfx/texture.cpp
    lib/gfx/movie_player/shader.cpp
    lib/gfx/textured_quad/shader.cpp
    lib/gfx/colored_quad/shader.cpp
    lib/gfx/stream.cpp
    lib/gfx/texture_convert.cpp
    )
target_compile_definitions(aurora PRIVATE IMGUI_USER_CONFIG="imconfig_user.h") # IMGUI_USE_WCHAR32
target_include_directories(aurora PUBLIC include ../)
target_include_directories(aurora PRIVATE ../imgui ../extern/imgui)
target_include_directories(aurora PRIVATE
    ../extern/dawn/src
    ../extern/dawn/third_party/abseil-cpp
    ${CMAKE_CURRENT_BINARY_DIR}/dawn/gen/src) # for hacks :)
target_link_libraries(aurora PRIVATE dawn_native dawncpp webgpu_dawn zeus logvisor SDL2-static xxhash)
if (APPLE)
  target_compile_definitions(aurora PRIVATE DAWN_ENABLE_BACKEND_METAL)
  target_sources(aurora PRIVATE lib/dawn/MetalBinding.mm)
  set_source_files_properties(lib/dawn/MetalBinding.mm PROPERTIES COMPILE_FLAGS -fobjc-arc)
elseif (WIN32)
  target_compile_definitions(aurora PRIVATE
      DAWN_ENABLE_BACKEND_D3D12
      DAWN_ENABLE_BACKEND_VULKAN)
  target_sources(aurora PRIVATE lib/dawn/D3D12Binding.cpp lib/dawn/VulkanBinding.cpp)
else ()
  target_compile_definitions(aurora PRIVATE
      DAWN_ENABLE_BACKEND_VULKAN
      DAWN_ENABLE_BACKEND_OPENGL
      DAWN_ENABLE_BACKEND_DESKTOP_GL)
  target_sources(aurora PRIVATE lib/dawn/OpenGLBinding.cpp lib/dawn/VulkanBinding.cpp)
endif ()
