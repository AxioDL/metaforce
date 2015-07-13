make_dnalist(liblist
             PAK
             MLVL
             STRG)
add_library(DNAMP3 ${liblist}
            PAK.cpp
            STRG.cpp)
