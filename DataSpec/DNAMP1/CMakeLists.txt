include(DNAMP1/ScriptObjects/CMakeLists.txt)
include(DNAMP1/SFX/CMakeLists.txt)

make_dnalist(PAK
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
             MazeSeeds
             SnowForces
             DCLN
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
             Tweaks/CTweakTargeting
             Tweaks/CTweakAutoMapper
             Tweaks/CTweakBall
             Tweaks/CTweakGuiColors)

set(DNAMP1_SOURCES
    DNAMP1.hpp DNAMP1.cpp
    AFSM.cpp
    PAK.cpp
    MLVL.cpp
    STRG.hpp STRG.cpp
    AGSC.cpp
    CSNG.cpp
    CSKR.cpp
    ANCS.cpp
    ANIM.cpp
    CINF.cpp
    EVNT.cpp
    PATH.cpp
    CMDL.hpp CMDL.cpp
    CMDLMaterials.cpp
    DCLN.cpp
    MAPA.hpp
    MAPU.hpp
    MREA.cpp
    SCLY.hpp SCLY.cpp
    FRME.cpp
    SCAN.cpp
    DeafBabe.cpp
    Tweaks/CTweakPlayer.cpp
    Tweaks/CTweakTargeting.cpp
    Tweaks/CTweakBall.cpp
    Tweaks/CTweakGame.cpp)

dataspec_add_list(DNAMP1 DNAMP1_SOURCES)
