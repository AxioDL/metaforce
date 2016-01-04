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
    ../DataSpecRegistry.hpp
)

if(NOT WIN32)
  list(APPEND PLAT_LIBS pthread)
endif()

target_link_libraries(hecl
    ${DATA_SPEC_LIBS}
    HECLDatabase HECLBackend HECLFrontend HECLBlender HECLCommon AthenaCore NOD
    LogVisor AthenaLibYaml ${PNG_LIB} squish xxhash ${ZLIB_LIBRARIES} ${LZO_LIB} ${PLAT_LIBS})
