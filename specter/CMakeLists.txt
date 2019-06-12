add_subdirectory(freetype2)
if (NOT MSVC)
  target_compile_options(freetype PRIVATE -Wno-implicit-fallthrough)
endif()
add_subdirectory(zeus)
set(ZEUS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zeus/include)

add_subdirectory(resources/fonts)

list(APPEND SPECTER_HEADERS
     include/specter/specter.hpp
     include/specter/ViewResources.hpp
     include/specter/IViewManager.hpp
     include/specter/View.hpp
     include/specter/RootView.hpp
     include/specter/ModalWindow.hpp
     include/specter/MessageWindow.hpp
     include/specter/Tooltip.hpp
     include/specter/SplitView.hpp
     include/specter/ScrollView.hpp
     include/specter/TextView.hpp
     include/specter/MultiLineTextView.hpp
     include/specter/Space.hpp
     include/specter/Toolbar.hpp
     include/specter/Table.hpp
     include/specter/Outliner.hpp
     include/specter/Panel.hpp
     include/specter/Control.hpp
     include/specter/Button.hpp
     include/specter/TextField.hpp
     include/specter/NumericField.hpp
     include/specter/Menu.hpp
     include/specter/IMenuNode.hpp
     include/specter/Node.hpp
     include/specter/NodeSocket.hpp
     include/specter/PathButtons.hpp
     include/specter/FileBrowser.hpp
     include/specter/Icon.hpp
     include/specter/FontCache.hpp
     include/specter/Translator.hpp
     include/specter/genie.hpp)

list(APPEND SPECTER_SOURCES
     lib/ViewResources.cpp
     lib/View.cpp
     lib/RootView.cpp
     lib/ModalWindow.cpp
     lib/MessageWindow.cpp
     lib/Tooltip.cpp
     lib/SplitView.cpp
     lib/ScrollView.cpp
     lib/TextView.cpp
     lib/MultiLineTextView.cpp
     lib/Space.cpp
     lib/Toolbar.cpp
     lib/Table.cpp
     lib/Control.cpp
     lib/Button.cpp
     lib/TextField.cpp
     lib/Menu.cpp
     lib/PathButtons.cpp
     lib/FileBrowser.cpp
     lib/Icon.cpp
     lib/FontCache.cpp
     lib/Translator.cpp)

add_library(specter ${SPECTER_SOURCES} ${SPECTER_HEADERS})
target_link_libraries(specter PUBLIC specter-fonts freetype hecl-full zeus)
target_include_directories(specter PUBLIC include freetype2/include)
target_atdna(specter atdna_FontCache.cpp include/specter/FontCache.hpp)
