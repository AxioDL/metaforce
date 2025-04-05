add_library(imgui_support
    ImGuiEngine.cpp
    ImGuiEngine.hpp
    NotoMono.cpp
    MetaforceIcon.cpp
    )
target_link_libraries(imgui_support PUBLIC aurora::core imgui ${ZLIB_LIBRARIES})
target_include_directories(imgui_support PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(imgui_support PRIVATE ${CMAKE_SOURCE_DIR})

bintoc_compress(NotoMono.cpp NotoMono-Regular.ttf NOTO_MONO_FONT)
bintoc(MetaforceIcon.cpp ../Runtime/platforms/freedesktop/256x256/apps/metaforce.png METAFORCE_ICON)
