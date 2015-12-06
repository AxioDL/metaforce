bintoc(en_US.c en_US.yaml L_en_US)
bintoc(en_GB.c en_GB.yaml L_en_GB)
add_library(RudeLocales
            en_US.yaml en_US.c
            en_GB.yaml en_GB.c
            locale.hpp locale.cpp)
