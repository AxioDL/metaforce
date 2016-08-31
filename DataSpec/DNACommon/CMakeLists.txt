make_dnalist(liblist
             CMDL
             MAPA
             EGMC
             SAVWCommon
             ParticleCommon)

add_library(DNACommon
            DNACommon.hpp DNACommon.cpp
            ${liblist}
            PAK.hpp PAK.cpp
            GX.hpp
            FSM2.hpp FSM2.cpp
            MLVL.hpp MLVL.cpp
            CMDL.cpp
            MAPA.cpp
            STRG.hpp STRG.cpp
            TXTR.hpp TXTR.cpp
            ANCS.hpp
            ANIM.hpp ANIM.cpp
            PART.hpp PART.cpp
            SWHC.hpp SWHC.cpp
            CRSC.hpp CRSC.cpp
            ELSC.hpp ELSC.cpp
            WPSC.hpp WPSC.cpp
            DPSC.hpp DPSC.cpp
            ParticleCommon.cpp
            FONT.hpp FONT.cpp
            DGRP.hpp DGRP.cpp
            DeafBabe.hpp DeafBabe.cpp
            BabeDead.hpp BabeDead.cpp
            RigInverter.hpp RigInverter.cpp
            AROTBuilder.hpp AROTBuilder.cpp
            Tweaks/TweakWriter.hpp
            Tweaks/ITweakGame.hpp
            Tweaks/ITweakParticle.hpp
            Tweaks/ITweakPlayer.hpp
            Tweaks/ITweakPlayerControl.hpp
            Tweaks/ITweakPlayerGun.hpp
            Tweaks/ITweakGunRes.hpp
            Tweaks/ITweakPlayerRes.hpp
            Tweaks/ITweakGui.hpp)
