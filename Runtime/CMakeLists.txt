include_directories(${BOO_INCLUDE_DIR})

add_subdirectory(Audio)
add_subdirectory(Character)
add_subdirectory(Graphics)
add_subdirectory(GuiSys)
add_subdirectory(Input)
add_subdirectory(Particle)

add_library(RuntimeCommon
            COsContext.hpp COsContextBoo.cpp
            CMainFlow.hpp CMainFlow.cpp
            CMemory.hpp CMemory.cpp
            CMemoryCardSys.hpp CMemoryCardSysPC.cpp
            IAllocator.hpp IAllocator.cpp
            CGameAllocator.hpp CGameAllocator.cpp
            CBasics.hpp CBasicsPC.cpp
            CIOWin.hpp CIOWin.cpp
            CIOWinManager.hpp CIOWinManager.cpp
            CStateManager.hpp CStateManager.cpp
            CGameState.hpp CGameState.cpp
            CScriptMailbox.hpp CScriptMailbox.cpp
            CMapWorldInfo.hpp CMapWorldInfo.cpp
            CPlayerState.hpp CPlayerState.cpp
            CWorldTransManager.hpp CWorldTransManager.cpp
            CRandom16.hpp CRandom16.cpp
            CResFactory.hpp CResFactory.cpp
            CFactoryStore.hpp CFactoryStore.cpp
            CSimplePool.hpp CSimplePool.cpp
            CAi.hpp CAi.cpp
            CGameOptions.hpp CGameOptions.cpp
            CStaticInterference.hpp CStaticInterference.cpp
            CCRC32.hpp CCRC32.cpp
            IFactory.hpp
            GameGlobalObjects.hpp
            RetroTemplates.hpp
            GCNTypes.hpp)

add_subdirectory(MP1)
add_subdirectory(MP2)
add_subdirectory(MP3)
