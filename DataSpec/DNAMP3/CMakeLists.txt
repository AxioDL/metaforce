list(APPEND types
     PAK)

make_dnalist(liblist ${types})
add_library(DNAMP3 ${liblist})
