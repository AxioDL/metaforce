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
    HECLDatabase
    "-Wl,-whole-archive" ${DATA_SPEC_LIBS} "-Wl,-no-whole-archive"
    HECL AthenaCore NOD LogVisor blowfish z lzo2 pthread
)
