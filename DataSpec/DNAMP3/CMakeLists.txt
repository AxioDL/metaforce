make_dnalist(liblist
             PAK
             MLVL
             ANIM
             CHAR
             CMDLMaterials
             CINF
             CSKR
             MREA
             SAVW
             CAUD)
add_library(DNAMP3
            DNAMP3.hpp DNAMP3.cpp
            ${liblist}
            PAK.cpp
            ANIM.cpp
            CINF.cpp
            CHAR.cpp
            CMDL.hpp
            CMDLMaterials.cpp
            STRG.hpp STRG.cpp
            MAPA.hpp
            MREA.cpp)
