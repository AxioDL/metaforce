include_directories(. ../Common)
add_executable(mp1
               CTweaks.hpp CTweaks.cpp
               CInGameTweakManager.hpp CInGameTweakManager.cpp
               CTweakParticle.hpp CTweakParticle.cpp
               main.cpp)
target_link_libraries(mp1
                      RuntimeCommonCharacter
                      RuntimeCommon
                      DNAMP1
                      DNACommon)
