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
                      RuntimeCommonGraphics
                      RuntimeCommon
                      DNAMP1
                      DNACommon
                      hecl-database hecl-blender hecl-common athena-core nod
                      logvisor athena-libyaml boo ${PNG_LIB} squish xxhash zeus
                      ${ZLIB_LIBRARIES} ${LZO_LIB}
                      ${BOO_SYS_LIBS})
