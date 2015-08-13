make_dnalist(liblist
             PAK
             MLVL
             CMDLMaterials
             CINF
             CSKR)
add_library(DNAMP3
            DNAMP3.hpp DNAMP3.cpp
            ${liblist}
            PAK.cpp
            CMDL.hpp
            CMDLMaterials.cpp
            STRG.hpp STRG.cpp)
