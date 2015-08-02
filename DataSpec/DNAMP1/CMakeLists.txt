make_dnalist(liblist
             PAK
             MLVL
             CMDL
             CMDLMaterials)
add_library(DNAMP1
            DNAMP1.hpp DNAMP1.cpp
            ${liblist}
            PAK.cpp
            STRG.hpp STRG.cpp
            CMDL.cpp
            CMDLMaterials.cpp)
