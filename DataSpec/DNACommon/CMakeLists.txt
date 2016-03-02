make_dnalist(liblist
             CMDL
             MAPA
             EGMC
             ParticleCommon)

add_library(DNACommon
            DNACommon.hpp DNACommon.cpp
            ${liblist}
            PAK.hpp PAK.cpp
            GX.hpp
            MLVL.hpp MLVL.cpp
            CMDL.cpp
            MAPA.cpp
            STRG.hpp STRG.cpp
            TXTR.hpp TXTR.cpp
            ANCS.hpp
            ANIM.hpp ANIM.cpp
            PART.hpp PART.cpp
            SWHC.hpp
            ELSC.hpp
            ParticleCommon.cpp
            DeafBabe.hpp
            BabeDead.hpp
            Tweaks/ITweakGame.hpp
            Tweaks/ITweakParticle.hpp
            Tweaks/ITweakPlayer.hpp
            Tweaks/ITweakPlayerControl.hpp)
