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
)

list(APPEND DATA_SPEC_LIBS
     RetroDataSpec
     DNACommon
     DNAMP1
     DNAMP2
     DNAMP3)

if(NOT WIN32)
set(WHOLE_START "-Wl,-whole-archive")
set(WHOLE_END "-Wl,-no-whole-archive")
endif()

target_link_libraries(hecl
    ${WHOLE_START} HECLDatabaseInit ${DATA_SPEC_LIBS} ${WHOLE_END}
    HECLDatabase HECLCommon AthenaCore AngelScript NOD LogVisor png squish blowfish z lzo2
)
