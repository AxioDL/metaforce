make_dnalist(liblist
             CMDL
             MAPA
             EGMC
             ParticleCommon)

add_library(DNACommon
            DNACommon.hpp DNACommon.cpp
            ${liblist}
            PAK.hpp
            GX.hpp
            MLVL.hpp
            CMDL.cpp
            STRG.hpp STRG.cpp
            TXTR.hpp TXTR.cpp
            ANCS.hpp
            ANIM.hpp ANIM.cpp
            PART.hpp
            SWHC.hpp
            ELSC.hpp
            ParticleCommon.cpp
            DeafBabe.hpp
            BabeDead.hpp
            IProperty.hpp
            Tweaks/ITweakGame.hpp
            Tweaks/ITweakParticle.hpp
            Tweaks/ITweakPlayer.hpp
            Tweaks/ITweakPlayerControl.hpp)
