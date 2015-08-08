make_dnalist(liblist
             PAK
             MLVL
             ANCS
             MAPA
             CMDL
             CMDLMaterials)
add_library(DNAMP1
            DNAMP1.hpp DNAMP1.cpp
            ${liblist}
            PAK.cpp
            STRG.hpp STRG.cpp
            ANCS.cpp
            CMDL.cpp
            CMDLMaterials.cpp)
