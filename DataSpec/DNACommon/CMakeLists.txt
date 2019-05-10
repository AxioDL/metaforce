make_dnalist(liblist DNACommon
             CMDL
             FONT
             DGRP
             FSM2
             MAPA
             MAPU
             EGMC
             SAVWCommon
             ParticleCommon
             Tweaks/ITweakPlayerGun)

set(DNACOMMON_SOURCES
    DNACommon.hpp DNACommon.cpp
    PAK.hpp PAK.cpp
    GX.hpp GX.cpp
    FSM2.hpp FSM2.cpp
    MLVL.hpp MLVL.cpp
    CMDL.cpp
    MAPA.cpp
    MAPU.cpp
    STRG.hpp STRG.cpp
    TXTR.hpp TXTR.cpp
    ANCS.hpp ANCS.cpp
    ANIM.hpp ANIM.cpp
    PART.hpp PART.cpp
    SWHC.hpp SWHC.cpp
    CRSC.hpp CRSC.cpp
    ELSC.hpp ELSC.cpp
    WPSC.hpp WPSC.cpp
    DPSC.hpp DPSC.cpp
    ParticleCommon.cpp
    FONT.cpp
    DGRP.cpp
    ATBL.hpp ATBL.cpp
    DeafBabe.hpp DeafBabe.cpp
    BabeDead.hpp BabeDead.cpp
    RigInverter.hpp RigInverter.cpp
    AROTBuilder.hpp AROTBuilder.cpp
    OBBTreeBuilder.hpp OBBTreeBuilder.cpp
    Tweaks/ITweak.hpp
    Tweaks/TweakWriter.hpp
    Tweaks/ITweakGame.hpp
    Tweaks/ITweakParticle.hpp
    Tweaks/ITweakPlayer.hpp
    Tweaks/ITweakPlayerControl.hpp
    Tweaks/ITweakGunRes.hpp
    Tweaks/ITweakPlayerRes.hpp
    Tweaks/ITweakGui.hpp
    Tweaks/ITweakSlideShow.hpp
    Tweaks/ITweakTargeting.hpp
    Tweaks/ITweakAutoMapper.hpp
    Tweaks/ITweakBall.hpp
    Tweaks/ITweakGuiColors.hpp)

dataspec_add_list(DNACommon DNACOMMON_SOURCES)
list(APPEND DNACOMMON_SOURCES ${liblist})
