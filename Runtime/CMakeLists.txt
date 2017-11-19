include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
                    ${BOO_INCLUDE_DIR} ${LIBJPEG_INCLUDE_DIR}
                    ${ZLIB_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/DataSpec ${JBUS_INCLUDE_DIR})

macro(runtime_add_list rel_path a_list)
  unset(tmp_list)
  foreach(path IN LISTS ${a_list})
    list(APPEND tmp_list "${rel_path}/${path}")
  endforeach(path)
  set(${a_list} "${tmp_list}" PARENT_SCOPE)
endmacro(runtime_add_list)

add_subdirectory(Audio)
add_subdirectory(Character)
add_subdirectory(Graphics)
add_subdirectory(Collision)
add_subdirectory(Camera)
add_subdirectory(World)
add_subdirectory(Weapon)
add_subdirectory(AutoMapper)
add_subdirectory(GuiSys)
add_subdirectory(Input)
add_subdirectory(Particle)

if(WIN32)
  list(APPEND PLAT_SRCS CMemoryCardSysWin.cpp)
elseif(APPLE)
  list(APPEND PLAT_SRCS CMemoryCardSysOSX.cpp)
else()
  list(APPEND PLAT_SRCS CMemoryCardSysNix.cpp)
endif()

add_custom_command(OUTPUT TCastTo.hpp TCastTo.cpp DEPENDS MkCastTo.py
                   COMMAND python ARGS ${CMAKE_CURRENT_SOURCE_DIR}/MkCastTo.py
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   COMMENT "Generating cast functions")

add_subdirectory(MP1)
add_subdirectory(MP2)
add_subdirectory(MP3)

if(APPLE)
  set_source_files_properties(MP1/CFrontEndUITouchBarMac.mm
                              MP1/CSaveGameScreenTouchBarMac.mm
                              CGameOptionsTouchBarMac.mm
                              PROPERTIES COMPILE_FLAGS -fobjc-arc)
  bintoc(startButton.c Resources/startButton@2x.png START_BUTTON_2X)
  list(APPEND PLAT_SRCS startButton.c CGameOptionsTouchBarMac.mm)
endif()

add_library(RuntimeCommon
            RetroTypes.hpp RetroTypes.cpp
            ${CLIENT_SOURCES}
            ${MP1_SOURCES}
            ${AUDIO_SOURCES}
            ${AUTOMAPPER_SOURCES}
            ${CAMERA_SOURCES}
            ${CHARACTER_SOURCES}
            ${COLLISION_SOURCES}
            ${GRAPHICS_SOURCES}
            ${GUISYS_SOURCES}
            ${INPUT_SOURCES}
            ${PARTICLE_SOURCES}
            ${WORLD_SOURCES}
            ${WEAPON_SOURCES}
            ITweak.hpp
            IMain.hpp
            CStopwatch.hpp
            CGameAllocator.hpp CGameAllocator.cpp
            CMemoryCardSys.hpp CMemoryCardSys.cpp
            CScannableObjectInfo.hpp CScannableObjectInfo.cpp
            CSaveWorld.hpp CSaveWorld.cpp
            CDependencyGroup.hpp CDependencyGroup.cpp
            CBasics.hpp CBasicsPC.cpp
            CIOWin.hpp
            CIOWinManager.hpp CIOWinManager.cpp
            CStateManager.hpp CStateManager.cpp
            CGameState.hpp CGameState.cpp
            CRelayTracker.hpp CRelayTracker.cpp
            CPlayerState.hpp CPlayerState.cpp
            CRandom16.hpp CRandom16.cpp
            CResFactory.hpp CResFactory.cpp
            CResLoader.hpp CResLoader.cpp
            CDvdRequest.hpp
            CDvdFile.hpp CDvdFile.cpp
            CVirtualDvdFile.hpp CVirtualDvdFile.cpp
            IObjectStore.hpp
            CSimplePool.hpp CSimplePool.cpp
            CGameOptions.hpp CGameOptions.cpp
            CGameOptionsTouchBar.hpp CGameOptionsTouchBar.cpp
            CStaticInterference.hpp CStaticInterference.cpp
            CCRC32.hpp CCRC32.cpp
            IFactory.hpp
            IObjFactory.hpp
            CObjectList.hpp CObjectList.cpp
            GameObjectLists.hpp GameObjectLists.cpp
            CSortedLists.hpp CSortedLists.cpp
            CArchitectureMessage.hpp
            CArchitectureQueue.hpp CArchitectureQueue.cpp
            IObj.hpp
            IVParamObj.hpp
            CTimeProvider.hpp CTimeProvider.cpp
            CToken.hpp CToken.cpp
            CFactoryMgr.hpp CFactoryMgr.cpp
            CPakFile.hpp CPakFile.cpp
            CStringExtras.hpp
            IOStreams.hpp IOStreams.cpp
            CMainFlowBase.hpp CMainFlowBase.cpp
            CMFGameBase.hpp
            CInGameTweakManagerBase.hpp
            CPlayMovieBase.hpp
            CGameDebug.hpp CGameDebug.cpp
            CGameHintInfo.hpp CGameHintInfo.cpp
            rstl.hpp
            GameGlobalObjects.hpp GameGlobalObjects.cpp
            MkCastTo.py
            TCastTo.hpp TCastTo.cpp
            GCNTypes.hpp
            ${PLAT_SRCS})
