make_dnalist(liblist
             PAK
             MLVL
             STRG)
add_library(DNAMP1
            DNAMP1.hpp DNAMP1.cpp
            ${liblist}
            PAK.cpp
            STRG.cpp)
