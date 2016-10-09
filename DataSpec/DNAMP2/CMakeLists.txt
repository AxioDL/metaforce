make_dnalist(liblist DNAMP2
             MLVL
             ANIM
             AGSC
             ANCS
             CMDLMaterials
             CINF
             CSKR
             MREA
             PTLA
             SAVW
             DeafBabe)
set(DNAMP2_SOURCES
    DNAMP2.hpp DNAMP2.cpp
    ANIM.cpp
    AGSC.cpp
    CINF.cpp
    ANCS.cpp
    CMDL.hpp
    MREA.cpp
    MAPA.hpp
    AFSM.hpp
    STRG.hpp STRG.cpp)

dataspec_add_list(DNAMP2 DNAMP2_SOURCES)
list(APPEND DNAMP2_SOURCES ${liblist})
