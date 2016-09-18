make_dnalist(liblist
             MLVL
             ANIM
             AGSC
             ANCS
             CMDLMaterials
             CINF
             CSKR
             MREA
             PTLA
             SAVW
             DeafBabe)
add_library(DNAMP2
            DNAMP2.hpp DNAMP2.cpp
            ${liblist}
            ANIM.cpp
            AGSC.cpp
            CINF.cpp
            ANCS.cpp
            CMDL.hpp
            MREA.cpp
            MAPA.hpp
            AFSM.hpp
            STRG.hpp STRG.cpp)
