if(NOT WINDOWS_STORE)
add_executable(hecl main.cpp
    ToolBase.hpp
    ToolPackage.hpp
    ToolExtract.hpp
    ToolInit.hpp
    ToolHelp.hpp
    ToolGroup.hpp
    ToolCook.hpp
    ToolClean.hpp
    ToolAdd.hpp
    ToolRemove.hpp
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

target_link_libraries(hecl
    ${DATA_SPEC_LIBS}
    hecl-common hecl-blender-addon athena-core nod
    logvisor athena-libyaml ${PNG_LIB} squish xxhash zeus boo
    ${ZLIB_LIBRARIES} ${LZO_LIB} ${PLAT_LIBS} ${BOO_SYS_LIBS})
endif()
