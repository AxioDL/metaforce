make_dnalist(liblist
             PAK
             MLVL
             ANCS
             MAPA
             CMDLMaterials)
add_library(DNAMP1
            DNAMP1.hpp DNAMP1.cpp
            ${liblist}
            PAK.cpp
            STRG.hpp STRG.cpp
            ANCS.cpp
            CMDL.hpp
            CMDLMaterials.cpp)
