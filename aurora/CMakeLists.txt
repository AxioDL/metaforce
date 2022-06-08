add_library(aurora STATIC
    lib/aurora.cpp
    lib/gpu.cpp
    lib/imgui.cpp
    lib/input.cpp
    lib/dawn/BackendBinding.cpp
    lib/gfx/common.cpp
    lib/gfx/texture.cpp
    lib/gfx/stream.cpp
    lib/gfx/gx.cpp
    lib/gfx/gx_shader.cpp
    lib/gfx/texture_convert.cpp
    lib/gfx/movie_player/shader.cpp
    lib/gfx/stream/shader.cpp
    lib/gfx/model/shader.cpp
    )
target_compile_definitions(aurora PRIVATE IMGUI_USER_CONFIG="imconfig_user.h") # IMGUI_USE_WCHAR32
target_include_directories(aurora PUBLIC include ../)
target_include_directories(aurora PRIVATE ../imgui ../extern/imgui)
if(NOT TARGET SDL2::SDL2-static)
  find_package(SDL2 REQUIRED)
endif()
target_link_libraries(aurora PRIVATE dawn_native dawncpp webgpu_dawn zeus logvisor SDL2::SDL2-static xxhash
    absl::btree absl::flat_hash_map)
if (DAWN_ENABLE_VULKAN)
  target_compile_definitions(aurora PRIVATE DAWN_ENABLE_BACKEND_VULKAN)
  target_sources(aurora PRIVATE lib/dawn/VulkanBinding.cpp)
endif ()
if (DAWN_ENABLE_METAL)
  target_compile_definitions(aurora PRIVATE DAWN_ENABLE_BACKEND_METAL)
  target_sources(aurora PRIVATE lib/dawn/MetalBinding.mm)
  set_source_files_properties(lib/dawn/MetalBinding.mm PROPERTIES COMPILE_FLAGS -fobjc-arc)
endif ()
if (DAWN_ENABLE_D3D12)
  target_compile_definitions(aurora PRIVATE DAWN_ENABLE_BACKEND_D3D12)
  target_sources(aurora PRIVATE lib/dawn/D3D12Binding.cpp)
endif ()
if (DAWN_ENABLE_DESKTOP_GL)
  target_compile_definitions(aurora PRIVATE DAWN_ENABLE_BACKEND_OPENGL DAWN_ENABLE_BACKEND_DESKTOP_GL)
  target_sources(aurora PRIVATE lib/dawn/OpenGLBinding.cpp)
endif ()
if (DAWN_ENABLE_NULL)
  target_compile_definitions(aurora PRIVATE DAWN_ENABLE_BACKEND_NULL)
  target_sources(aurora PRIVATE lib/dawn/NullBinding.cpp)
endif ()
