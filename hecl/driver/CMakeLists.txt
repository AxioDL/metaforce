if(NOT WINDOWS_STORE)

add_executable(hecl main.cpp
    ToolBase.hpp
    ToolPackage.hpp
    ToolExtract.hpp
    ToolInit.hpp
    ToolHelp.hpp
    ToolCook.hpp
    ToolImage.hpp
    ToolSpec.hpp
    ../DataSpecRegistry.hpp)
if(COMMAND add_sanitizers)
  add_sanitizers(hecl)
endif()

if(NOT WIN32)
  list(APPEND PLAT_LIBS pthread)
endif()

if(APPLE)
  find_library(CF_LIBRARY CoreFoundation)
  list(APPEND PLAT_LIBS ${CF_LIBRARY})
endif()

target_link_libraries(hecl PUBLIC ${DATA_SPEC_LIBS} hecl-full)

if(TARGET nod)
  target_link_libraries(hecl PUBLIC nod)
  target_compile_definitions(hecl PUBLIC HECL_HAS_NOD=1)
endif()

target_compile_definitions(hecl PRIVATE HECL_DLPACKAGE="${HECL_DLPACKAGE}")

endif()
