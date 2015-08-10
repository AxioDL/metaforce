make_dnalist(liblist
             PAK
             MLVL
             CMDLMaterials)
add_library(DNAMP3
            DNAMP3.hpp DNAMP3.cpp
            ${liblist}
            PAK.cpp
            CMDL.hpp
            CMDLMaterials.cpp
            STRG.hpp STRG.cpp)
