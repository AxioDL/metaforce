include(DNAMP1/ScriptObjects/CMakeLists.txt)

make_dnalist(liblist DNAMP1
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
             Tweaks/CTweakCameraBob
             Tweaks/CTweakTargeting)

set(DNAMP1_SOURCES
    DNAMP1.hpp DNAMP1.cpp
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
    Tweaks/CTweakTargeting.cpp)

dataspec_add_list(DNAMP1 DNAMP1_SOURCES)
list(APPEND DNAMP1_SOURCES ${liblist})
