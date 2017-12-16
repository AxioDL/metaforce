add_subdirectory(freetype2)
add_definitions("-DZE_ATHENA_TYPES=1")
add_subdirectory(zeus)
set(ZEUS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zeus/include)

if(NOT DEFINED HECL_INCLUDE_DIR)
  message(FATAL_ERROR "Specter may only be built as a sub-project containing hecl with
                       HECL_INCLUDE_DIR set")
endif()

if(NOT DEFINED BOO_INCLUDE_DIR)
  message(FATAL_ERROR "Specter may only be built as a sub-project containing boo with
                       BOO_INCLUDE_DIR set")
endif()

if(NOT DEFINED ATHENA_INCLUDE_DIR)
  message(FATAL_ERROR "Specter may only be built as a sub-project containing Athena with
                       ATHENA_INCLUDE_DIR set")
endif()

if(NOT DEFINED LOGVISOR_INCLUDE_DIR)
  message(FATAL_ERROR "Specter may only be built as a sub-project containing logvisor with
                       LOGVISOR_INCLUDE_DIR set")
endif()

add_subdirectory(resources/fonts)

include_directories(include ${HECL_INCLUDE_DIR} ${BOO_INCLUDE_DIR}
                    ${LOGVISOR_INCLUDE_DIR} ${ATHENA_INCLUDE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/freetype2/include
                    ${ZEUS_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

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

atdna(atdna_FontCache.cpp include/specter/FontCache.hpp)

list(APPEND SPECTER_SOURCES
     lib/specter.cpp
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
     lib/Outliner.cpp
     lib/Panel.cpp
     lib/Control.cpp
     lib/Button.cpp
     lib/TextField.cpp
     lib/NumericField.cpp
     lib/Menu.cpp
     lib/Node.cpp
     lib/NodeSocket.cpp
     lib/PathButtons.cpp
     lib/FileBrowser.cpp
     lib/Icon.cpp
     lib/FontCache.cpp
     lib/Translator.cpp
     atdna_FontCache.cpp)

add_library(specter ${SPECTER_SOURCES} ${SPECTER_HEADERS})
