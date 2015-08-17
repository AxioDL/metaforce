add_subdirectory(Graphics)
add_subdirectory(Audio)
add_subdirectory(Character)
add_subdirectory(GuiSys)
add_subdirectory(Script)

add_executable(RuntimeCheck
               main.hpp main.cpp
               COsContext.hpp COsContextBoo.cpp
               CMemory.hpp CMemory.cpp
               CMemoryCardSys.hpp CMemoryCardSysPC.cpp
               IAllocator.hpp IAllocator.cpp
               CGameAllocator.hpp CGameAllocator.cpp
               CBasics.hpp CBasicsPC.cpp
               CTweaks.hpp CTweaks.cpp
               CTweakParticle.hpp CTweakParticle.cpp
               CStateManager.hpp CStateManager.cpp
               CGameState.hpp CGameState.cpp
               CScriptMailbox.hpp CScriptMailbox.cpp
               CMapWorldInfo.hpp CMapWorldInfo.cpp
               CPlayerState.hpp CPlayerState.cpp
               CWorldTransManager.hpp CWorldTransManager.cpp
               CRandom16.hpp CRandom16.cpp
               CCRC32.hpp CCRC32.cpp)
