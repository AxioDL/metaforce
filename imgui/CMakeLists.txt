add_library(imgui
        ../extern/imgui/imgui.cpp
        ../extern/imgui/imgui_demo.cpp
        ../extern/imgui/imgui_draw.cpp
        ../extern/imgui/imgui_tables.cpp
        ../extern/imgui/imgui_widgets.cpp
        ../extern/imgui/backends/imgui_impl_vulkan.cpp
        ImGuiEngine.cpp
        ImGuiEngine.hpp
        )
target_include_directories(imgui PUBLIC ../extern/imgui ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(imgui PUBLIC IMGUI_USER_CONFIG="imconfig_user.h")
target_link_libraries(imgui PRIVATE boo hecl-light RetroDataSpec)

add_shader(ImguiShader)
target_link_libraries(shader_ImguiShader PRIVATE hecl-light)
