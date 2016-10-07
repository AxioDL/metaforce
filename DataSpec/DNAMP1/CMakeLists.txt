add_subdirectory(ScriptObjects)

make_dnalist(liblist
             PAK
             MLVL
             AGSC
             CSNG
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
             Tweaks/CTweakGui
             Tweaks/CTweakSlideShow
             Tweaks/CTweakCameraBob)

set(DNAMP1_SOURCES
    DNAMP1.hpp DNAMP1.cpp
    ${liblist}
    PAK.cpp
    MLVL.cpp
    STRG.hpp STRG.cpp
    AGSC.cpp
    CSNG.cpp
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
    DeafBabe.cpp
    Tweaks/CTweakPlayer.cpp
    ${ScriptObjectsMP1_SOURCES})

dataspec_add_list(DNAMP1 DNAMP1_SOURCES)
