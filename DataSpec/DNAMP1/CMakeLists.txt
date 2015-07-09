list(APPEND types
     PAK
     MLVL)

make_dnalist(liblist ${types})
add_library(DNAMP1 ${liblist})
