make_dnalist(liblist
             MLVL
             ANIM
             ANCS
             CMDLMaterials
             CINF
             CSKR)
add_library(DNAMP2
            DNAMP2.hpp DNAMP2.cpp
            ${liblist}
            ANIM.cpp
            CMDL.hpp
            STRG.hpp STRG.cpp)
