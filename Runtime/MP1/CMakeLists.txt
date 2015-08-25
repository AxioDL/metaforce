include_directories(. ..)
add_executable(mp1
               CTweaks.hpp CTweaks.cpp
               CInGameTweakManager.hpp CInGameTweakManager.cpp
               CTweakParticle.hpp CTweakParticle.cpp
               main.cpp)
target_link_libraries(mp1
                      RuntimeCommonCharacter
                      RuntimeCommon
                      DNAMP1
                      DNACommon
                      HECLDatabase HECLBlender HECLCommon AthenaCore NOD
                      LogVisor AthenaLibYaml Boo png squish blowfish z lzo2 pthread
                      ${BOO_SYS_LIBS})
