add_subdirectory(../extern/dawn dawn EXCLUDE_FROM_ALL)
if (NOT MSVC)
  target_compile_options(SPIRV-Tools-static PRIVATE -Wno-implicit-fallthrough)
  target_compile_options(SPIRV-Tools-opt PRIVATE -Wno-implicit-fallthrough)
endif ()

add_library(aurora STATIC lib/aurora.cpp lib/imgui.cpp lib/gfx/common.cpp)
target_include_directories(aurora PUBLIC include ../Runtime)
target_include_directories(aurora PRIVATE ../imgui ../extern/imgui)
target_link_libraries(aurora PRIVATE webgpu_dawn zeus logvisor) # TODO remove logvisor from rstl.hpp
