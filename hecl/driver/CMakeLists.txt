if(WIN32)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCLUDE:HECLDataSpecs")
endif()

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

list(APPEND DATA_SPEC_LIBS
     RetroDataSpec
     DNAMP3
     DNAMP2
     DNAMP1
     DNACommon)

target_link_libraries(hecl
    ${DATA_SPEC_LIBS}
    HECLDatabase HECLBlender HECLCommon AthenaCore NOD
    LogVisor AthenaLibYaml png squish blowfish z lzo2 pthread
)
