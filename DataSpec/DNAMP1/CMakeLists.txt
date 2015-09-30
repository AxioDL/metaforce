add_subdirectory(ScriptObjects)

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
             DeafBabe
             SCAN
             Tweaks/CTweakParticle
             Tweaks/CTweakPlayer
             Tweaks/CTweakPlayerControl)

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
            MREA.cpp
            SCLY.hpp SCLY.cpp
            DeafBabe.cpp)

target_link_libraries(DNAMP1 ScriptObjectsMP1)
