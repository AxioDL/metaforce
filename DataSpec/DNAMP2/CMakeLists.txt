make_dnalist(liblist
             MLVL
             ANIM
             ANCS
             CMDLMaterials
             CINF
             CSKR
             MREA
             PTLA
             DeafBabe)
add_library(DNAMP2
            DNAMP2.hpp DNAMP2.cpp
            ${liblist}
            ANIM.cpp
            CINF.cpp
            ANCS.cpp
            CMDL.hpp
            MREA.cpp
            MAPA.hpp
            AFSM.hpp
            STRG.hpp STRG.cpp)
