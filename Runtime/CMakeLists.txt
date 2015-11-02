include_directories(${BOO_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

add_subdirectory(Audio)
add_subdirectory(Character)
add_subdirectory(Graphics)
add_subdirectory(GuiSys)
add_subdirectory(Input)
add_subdirectory(Particle)

if(WIN32)
  list(APPEND PLAT_SRCS CMemoryCardSysWin.cpp)
elseif(APPLE)
  list(APPEND PLAT_SRCS CMemoryCardSysOSX.cpp)
else()
  list(APPEND PLAT_SRCS CMemoryCardSysNix.cpp CCallStackNix.cpp)
endif()

add_library(RuntimeCommon
            CMemory.hpp CMemory.cpp
            CMemoryCardSys.hpp
            IAllocator.hpp IAllocator.cpp
            CGameAllocator.hpp CGameAllocator.cpp
            CBasics.hpp CBasicsPC.cpp
            CIOWin.hpp
            CIOWinManager.hpp CIOWinManager.cpp
            CStateManager.hpp CStateManager.cpp
            CGameState.hpp CGameState.cpp
            CScriptMailbox.hpp CScriptMailbox.cpp
            CMapWorldInfo.hpp CMapWorldInfo.cpp
            CPlayerState.hpp CPlayerState.cpp
            CWorldTransManager.hpp CWorldTransManager.cpp
            CRandom16.hpp CRandom16.cpp
            CResFactory.hpp CResFactory.cpp
            CResLoader.hpp CResLoader.cpp
            CDvdRequest.hpp CNODDvdRequest.cpp
            CDvdFile.hpp CNODDvdFile.cpp
            CVirtualDvdFile.hpp CVirtualDvdFile.cpp
            IObjectStore.hpp
            CSimplePool.hpp CSimplePool.cpp
            CAi.hpp CAi.cpp
            CGameOptions.hpp CGameOptions.cpp
            CStaticInterference.hpp
            CCRC32.hpp CCRC32.cpp
            CEntity.hpp CEntity.cpp
            IFactory.hpp
            ScriptObjectSupport.hpp ScriptObjectSupport.cpp
            CObjectList.hpp
            CArchitectureMessage.hpp
            CArchitectureQueue.hpp CArchitectureQueue.cpp
            IObj.hpp
            IVParamObj.hpp
            CTimeProvider.hpp CTimeProvider.cpp
            CToken.hpp
            CAreaOctTree.hpp CAreaOctTree.cpp
            CActor.hpp CActor.cpp
            CPhysicsActor.hpp CPhysicsActor.cpp
            CFactoryMgr.hpp CFactoryMgr.cpp
            CPakFile.hpp CPakFile.cpp
            CStringExtras.hpp
            CCallStack.hpp
            IOStreams.hpp IOStreams.cpp
            CMainFlowBase.hpp CMainFlowBase.cpp
            CMFGameBase.hpp
            CInGameTweakManagerBase.hpp
            CPlayMovieBase.hpp
            CMoviePlayer.hpp CMoviePlayer.cpp
            CGameDebug.hpp CGameDebug.cpp
            rstl.hpp rstl.cpp
            GameGlobalObjects.hpp
            RetroTypes.hpp
            GCNTypes.hpp
            ${PLAT_SRCS})

add_subdirectory(MP1)
add_subdirectory(MP2)
add_subdirectory(MP3)
