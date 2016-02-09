include_directories(. ..)
add_executable(mp1
               CTweaks.hpp CTweaks.cpp
               CInGameTweakManager.hpp CInGameTweakManager.cpp
               CMainFlow.hpp CMainFlow.cpp
               CMFGame.hpp CMFGame.cpp
               CPlayMovie.hpp CPlayMovie.cpp
               CFrontEndUI.hpp CFrontEndUI.cpp
               CMain.hpp
               main.cpp)
target_link_libraries(mp1
                      RuntimeCommonCharacter
                      RuntimeCommonInput
                      RuntimeCommonParticle
                      RuntimeCommon
                      DNAMP1
                      DNACommon
                      HECLDatabase HECLBlender HECLCommon AthenaCore NOD
                      LogVisor AthenaLibYaml Boo ${PNG_LIB} squish xxhash Math
                      ${ZLIB_LIBRARIES} ${LZO_LIB}
                      ${BOO_SYS_LIBS})
