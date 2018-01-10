if(NOT WINDOWS_STORE)
add_executable(hecl main.cpp
    ToolBase.hpp
    ToolPackage.hpp
    ToolExtract.hpp
    ToolInit.hpp
    ToolHelp.hpp
    ToolCook.hpp
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
    hecl-full hecl-blender-addon athena-core nod
    logvisor athena-libyaml ${PNG_LIB} squish xxhash zeus boo
    ${ZLIB_LIBRARIES} ${LZO_LIB} ${PLAT_LIBS} ${BOO_SYS_LIBS})

set_target_properties(hecl PROPERTIES COMPILE_DEFINITIONS HECL_DLPACKAGE="${HECL_DLPACKAGE}")

if(COMMAND cotire)
  set_target_properties(hecl PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
  cotire(hecl)
endif()
endif()
