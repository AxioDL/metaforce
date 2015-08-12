make_dnalist(liblist
             MLVL
             CMDLMaterials)
add_library(DNAMP2
            DNAMP2.hpp DNAMP2.cpp
            ${liblist}
            CMDL.hpp
            STRG.hpp STRG.cpp)
