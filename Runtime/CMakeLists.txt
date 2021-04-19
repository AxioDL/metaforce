macro(runtime_add_list rel_path a_list)
  unset(tmp_list)
  foreach(path IN LISTS ${a_list})
    if(IS_ABSOLUTE ${path})
      list(APPEND tmp_list "${path}")
    else()
      list(APPEND tmp_list "${rel_path}/${path}")
    endif()
  endforeach(path)
  set(${a_list} "${tmp_list}" PARENT_SCOPE)
endmacro(runtime_add_list)

macro(runtime_add_hsh rel_path)
  set(tmp_list ${HSH_SOURCES})
  foreach(path ${ARGN})
    if(IS_ABSOLUTE ${path})
      list(APPEND tmp_list "${path}")
    else()
      list(APPEND tmp_list "${rel_path}/${path}")
    endif()
  endforeach(path)
  set(HSH_SOURCES ${tmp_list} PARENT_SCOPE)
endmacro(runtime_add_hsh)

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

hsh_sources(${HSH_SOURCES})

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

set(RUNTIME_SOURCES_A
    RetroTypes.hpp RetroTypes.cpp
    ${CLIENT_SOURCES}
    ${MP1_SOURCES}
    ${AUDIO_SOURCES}
    ${AUTOMAPPER_SOURCES}
    ${CAMERA_SOURCES}
    ${CHARACTER_SOURCES}
    ${COLLISION_SOURCES}
    ${GRAPHICS_SOURCES})

set(RUNTIME_SOURCES_B
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
    IObjectStore.hpp
    CSimplePool.hpp CSimplePool.cpp
    CGameOptions.hpp CGameOptions.cpp
    CStaticInterference.hpp CStaticInterference.cpp
    CCRC32.hpp CCRC32.cpp
    IFactory.hpp
    IObjFactory.hpp
    CObjectList.hpp CObjectList.cpp
    GameObjectLists.hpp GameObjectLists.cpp
    CSortedLists.hpp CSortedLists.cpp
    CArchitectureMessage.hpp
    CArchitectureQueue.hpp
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
    CGameDebug.hpp
    CGameHintInfo.hpp CGameHintInfo.cpp
    rstl.hpp
    GameGlobalObjects.hpp GameGlobalObjects.cpp
    MkCastTo.py
    TCastTo.hpp TCastTo.cpp
    GCNTypes.hpp
    CTextureCache.hpp CTextureCache.cpp
    ${PLAT_SRCS})

function(add_runtime_common_library name)
add_library(${name} ${ARGN})
if(COMMAND add_sanitizers)
  add_sanitizers(${name})
endif()

if(WINDOWS_STORE)
  set_property(TARGET ${name} PROPERTY VS_WINRT_COMPONENT TRUE)
endif()
endfunction()

set(RUNTIME_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
set(RUNTIME_LIBRARIES RetroDataSpec AssetNameMapNull NESEmulator
        libjpeg-turbo jbus kabufuda logvisor)
if(NOT GEKKO AND NOT NX)
  list(APPEND RUNTIME_LIBRARIES discord-rpc OptickCore)
endif()

add_runtime_common_library(RuntimeCommon ${RUNTIME_SOURCES_A})
add_runtime_common_library(RuntimeCommonB ${RUNTIME_SOURCES_B})
target_include_directories(RuntimeCommonB PUBLIC ${RUNTIME_INCLUDES})
target_link_libraries(RuntimeCommonB PUBLIC ${RUNTIME_LIBRARIES})
target_link_libraries(RuntimeCommon PUBLIC RuntimeCommonB)
target_hsh(RuntimeCommon)
target_hsh(RuntimeCommonB)

add_executable(urde CMain.hpp CMain.cpp)
target_link_libraries(urde PUBLIC RuntimeCommon)

if(NX)
  add_nro_target(urde)
  add_nso_target(urde)
endif()
