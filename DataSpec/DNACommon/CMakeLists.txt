make_dnalist(liblist
             CMDL)
add_library(DNACommon
            DNACommon.hpp DNACommon.cpp
            ${liblist}
            PAK.hpp
            GX.hpp
            STRG.hpp STRG.cpp
            TXTR.hpp TXTR.cpp
            ANCS.hpp
            ANIM.hpp ANIM.cpp
            Tweaks/ITweakPlayer.hpp
            Tweaks/ITweakPlayerControl.hpp)
