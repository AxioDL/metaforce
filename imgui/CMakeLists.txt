add_library(imgui
        ../extern/imgui/imgui.cpp
        ../extern/imgui/imgui_demo.cpp
        ../extern/imgui/imgui_draw.cpp
        ../extern/imgui/imgui_tables.cpp
        ../extern/imgui/imgui_widgets.cpp
        ImGuiEngine.cpp
        ImGuiEngine.hpp
        NotoMono.cpp
        )
target_include_directories(imgui PUBLIC ../extern/imgui ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(imgui PUBLIC IMGUI_USER_CONFIG="imconfig_user.h")
target_link_libraries(imgui PRIVATE boo hecl-light RetroDataSpec)

bintoc_compress(NotoMono.cpp NotoMono-Regular.ttf NOTO_MONO_FONT)

add_shader(ImGuiShader)
target_link_libraries(shader_ImGuiShader PRIVATE hecl-light)
