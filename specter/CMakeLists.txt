add_subdirectory(freetype2)

if(NOT DEFINED HECL_INCLUDE_DIR)
  message(FATAL_ERROR "Specter may only be built as a sub-project containing hecl with
                       HECL_INCLUDE_DIR set")
endif()

if(NOT DEFINED BOO_INCLUDE_DIR)
  message(FATAL_ERROR "Specter may only be built as a sub-project containing boo with
                       BOO_INCLUDE_DIR set")
endif()

include_directories(include ${HECL_INCLUDE_DIR} ${BOO_INCLUDE_DIR})

list(APPEND SPECTER_HEADERS
     include/Specter/Specter.hpp
     include/Specter/View.hpp
     include/Specter/RootView.hpp
     include/Specter/Space.hpp
     include/Specter/Table.hpp
     include/Specter/Outliner.hpp
     include/Specter/Panel.hpp
     include/Specter/Control.hpp
     include/Specter/Button.hpp
     include/Specter/TextField.hpp
     include/Specter/NumericField.hpp
     include/Specter/Menu.hpp
     include/Specter/Node.hpp
     include/Specter/NodeSocket.hpp)

list(APPEND SPECTER_SOURCES
     lib/Specter.cpp
     lib/View.cpp
     lib/RootView.cpp
     lib/Space.cpp
     lib/Table.cpp
     lib/Outliner.cpp
     lib/Panel.cpp
     lib/Control.cpp
     lib/Button.cpp
     lib/TextField.cpp
     lib/NumericField.cpp
     lib/Menu.cpp
     lib/Node.cpp
     lib/NodeSocket.cpp)

add_library(Specter ${SPECTER_SOURCES} ${SPECTER_HEADERS})
