add_subdirectory(freetype2)
add_subdirectory(MathLib)
set(MATHLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/MathLib/include)

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

if(NOT DEFINED LOG_VISOR_INCLUDE_DIR)
  message(FATAL_ERROR "Specter may only be built as a sub-project containing LogVisor with
                       LOG_VISOR_INCLUDE_DIR set")
endif()

add_subdirectory(resources/fonts)

include_directories(include ${HECL_INCLUDE_DIR} ${BOO_INCLUDE_DIR}
                    ${LOG_VISOR_INCLUDE_DIR} ${ATHENA_INCLUDE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/freetype2/include
                    ${MATHLIB_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

list(APPEND SPECTER_HEADERS
     include/Specter/Specter.hpp
     include/Specter/ViewResources.hpp
     include/Specter/IViewManager.hpp
     include/Specter/DeferredWindowEvents.hpp
     include/Specter/View.hpp
     include/Specter/RootView.hpp
     include/Specter/ModalWindow.hpp
     include/Specter/Tooltip.hpp
     include/Specter/SplitView.hpp
     include/Specter/ScrollView.hpp
     include/Specter/TextView.hpp
     include/Specter/MultiLineTextView.hpp
     include/Specter/Space.hpp
     include/Specter/Toolbar.hpp
     include/Specter/Table.hpp
     include/Specter/Outliner.hpp
     include/Specter/Panel.hpp
     include/Specter/Control.hpp
     include/Specter/Button.hpp
     include/Specter/TextField.hpp
     include/Specter/NumericField.hpp
     include/Specter/Menu.hpp
     include/Specter/Node.hpp
     include/Specter/NodeSocket.hpp
     include/Specter/FileBrowser.hpp
     include/Specter/FontCache.hpp
     include/Specter/Translator.hpp)

atdna(atdna_FontCache.cpp include/Specter/FontCache.hpp)

list(APPEND SPECTER_SOURCES
     lib/Specter.cpp
     lib/ViewResources.cpp
     lib/View.cpp
     lib/RootView.cpp
     lib/ModalWindow.cpp
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
     lib/FileBrowser.cpp
     lib/FontCache.cpp
     lib/Translator.cpp
     atdna_FontCache.cpp)

add_library(Specter ${SPECTER_SOURCES} ${SPECTER_HEADERS})
