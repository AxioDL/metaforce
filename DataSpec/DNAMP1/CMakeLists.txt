add_subdirectory(Tweaks)
make_dnalist(liblist
             PAK
             MLVL
             ANCS
             ANIM
             CINF
             CSKR
             EVNT
             MAPA
             CMDLMaterials
             MREA
             Tweaks/CTweakParticle)
add_library(DNAMP1
            DNAMP1.hpp DNAMP1.cpp
            ${liblist}
            PAK.cpp
            STRG.hpp STRG.cpp
            ANCS.cpp
            ANIM.cpp
            EVNT.cpp
            CMDL.hpp
            CMDLMaterials.cpp
            MREA.cpp)
