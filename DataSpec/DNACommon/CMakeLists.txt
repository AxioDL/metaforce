make_dnalist(liblist
             CMDL)
add_library(DNACommon
            DNACommon.hpp DNACommon.cpp
            ${liblist}
            PAK.hpp
            GX.hpp
            CMDL.cpp
            STRG.hpp STRG.cpp
            TXTR.hpp TXTR.cpp
            ANCS.hpp
            ANIM.hpp ANIM.cpp
            BabeDead.hpp
            Tweaks/ITweakPlayer.hpp
            Tweaks/ITweakPlayerControl.hpp)
