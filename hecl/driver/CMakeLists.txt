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
target_link_libraries(hecl HECL HECLDatabase
    "-Wl,-whole-archive" RetroDataSpec "-Wl,-no-whole-archive"
    AthenaCore NOD LogVisor blowfish zlib lzo pthread
)
