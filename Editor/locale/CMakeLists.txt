bintoc(en_US.cpp en_US.yaml L_en_US)
bintoc(en_GB.cpp en_GB.yaml L_en_GB)
bintoc(ja_JP.cpp ja_JP.yaml L_ja_JP)
add_library(UrdeLocales
            en_US.yaml en_US.cpp
            en_GB.yaml en_GB.cpp
            ja_JP.yaml ja_JP.cpp
            locale.hpp locale.cpp)
target_link_libraries(UrdeLocales specter)
