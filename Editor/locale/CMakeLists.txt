bintoc(en_US.c en_US.yaml L_en_US)
bintoc(en_GB.c en_GB.yaml L_en_GB)
bintoc(ja_JP.c ja_JP.yaml L_ja_JP)
add_library(RudeLocales
            en_US.yaml en_US.c
            en_GB.yaml en_GB.c
            ja_JP.yaml ja_JP.c
            locale.hpp locale.cpp)
