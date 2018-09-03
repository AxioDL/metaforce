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
    DeafBabe.cpp
    PAK.cpp
    ANIM.cpp
    AGSC.cpp
    CINF.cpp
    CSKR.cpp
    CMDL.cpp
    ANCS.cpp
    CMDL.hpp
    MREA.cpp
    MAPA.hpp
    MAPU.hpp
    AFSM.hpp
    STRG.hpp STRG.cpp)

dataspec_add_list(DNAMP2 DNAMP2_SOURCES)
list(APPEND DNAMP2_SOURCES ${liblist})
