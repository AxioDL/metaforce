add_subdirectory(ScriptObjects)

make_dnalist(liblist
             PAK
             MLVL
             AFSM
             ANCS
             ANIM
             CINF
             CSKR
             EVNT
             PATH
             CMDLMaterials
             MREA
             DeafBabe
             SCAN
             FRME
             SAVW
             HINT
             Tweaks/CTweakGame
             Tweaks/CTweakParticle
             Tweaks/CTweakPlayer
             Tweaks/CTweakPlayerControl
             Tweaks/CTweakPlayerGun
             Tweaks/CTweakGunRes
             Tweaks/CTweakPlayerRes
             Tweaks/CTweakGui)

add_library(DNAMP1
            DNAMP1.hpp DNAMP1.cpp
            ${liblist}
            PAK.cpp
            STRG.hpp STRG.cpp
            ANCS.cpp
            ANIM.cpp
            CINF.cpp
            EVNT.cpp
            CMDL.hpp
            CMDLMaterials.cpp
            MAPA.hpp
            MREA.cpp
            SCLY.hpp SCLY.cpp
            FRME.cpp
            DeafBabe.cpp)
