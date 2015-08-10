make_dnalist(liblist
             CMDL)
add_library(DNACommon
            DNACommon.hpp DNACommon.cpp
            ${liblist}
            GX.hpp
            STRG.hpp STRG.cpp
            TXTR.hpp TXTR.cpp)
