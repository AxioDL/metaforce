make_dnalist(liblist
             PAK
             MLVL)
add_library(DNAMP3
            DNAMP3.hpp DNAMP3.cpp
            ${liblist}
            PAK.cpp
            STRG.hpp STRG.cpp)
