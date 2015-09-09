make_dnalist(liblist
             MLVL
             ANIM
             ANCS
             CMDLMaterials
             CINF
             CSKR
             MREA)
add_library(DNAMP2
            DNAMP2.hpp DNAMP2.cpp
            ${liblist}
            ANIM.cpp
            ANCS.cpp
            CMDL.hpp
            MREA.cpp
            STRG.hpp STRG.cpp)
