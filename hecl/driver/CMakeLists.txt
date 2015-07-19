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
)

list(APPEND DATA_SPEC_LIBS
     RetroDataSpec
     DNACommon
     DNAMP1
     DNAMP2
     DNAMP3)

target_link_libraries(hecl
    "-Wl,-whole-archive" HECLDatabaseInit ${DATA_SPEC_LIBS} "-Wl,-no-whole-archive"
    HECLDatabase HECL AthenaCore AngelScript NOD LogVisor png squish blowfish z lzo2
)
