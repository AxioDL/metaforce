make_dnalist(liblist
             MLVL
             STRG)
add_library(DNAMP2
            DNAMP2.hpp
            DNAMP2.cpp
            ${liblist}
            STRG.cpp)
