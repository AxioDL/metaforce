make_dnalist(liblist
             PAK
             MLVL
             STRG)
add_library(DNAMP3
            DNAMP3.hpp DNAMP3.cpp
            ${liblist}
            PAK.cpp
            STRG.cpp)
