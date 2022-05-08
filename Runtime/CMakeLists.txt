macro(runtime_add_list rel_path a_list)
    unset(tmp_list)
    foreach (path IN LISTS ${a_list})
        if (IS_ABSOLUTE ${path})
            list(APPEND tmp_list "${path}")
        else ()
            list(APPEND tmp_list "${rel_path}/${path}")
        endif ()
    endforeach (path)
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

if (WIN32)
    list(APPEND PLAT_SRCS CMemoryCardSysWin.cpp)
else ()
    list(APPEND PLAT_SRCS CMemoryCardSysNix.cpp)
endif ()

add_custom_command(OUTPUT TCastTo.hpp TCastTo.cpp DEPENDS MkCastTo.py
        COMMAND python ARGS ${CMAKE_CURRENT_SOURCE_DIR}/MkCastTo.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating cast functions")

add_subdirectory(MP1)
add_subdirectory(MP2)
add_subdirectory(MP3)

set(CAST_TO_SOURCES
        MkCastTo.py
        TCastTo.hpp TCastTo.cpp)

set(RUNTIME_SOURCES_A
        RetroTypes.hpp RetroTypes.cpp
        ${CAST_TO_SOURCES}
        ${MP1_SOURCES}
        ${AUDIO_SOURCES}
        ${AUTOMAPPER_SOURCES}
        ${CAMERA_SOURCES}
        ${CHARACTER_SOURCES}
        ${COLLISION_SOURCES}
        ${GRAPHICS_SOURCES})

set(RUNTIME_SOURCES_B
        ${CAST_TO_SOURCES}
        ${GUISYS_SOURCES}
        ${INPUT_SOURCES}
        ${PARTICLE_SOURCES}
        ${WORLD_SOURCES}
        ${WEAPON_SOURCES}
        CInfiniteLoopDetector.hpp CInfiniteLoopDetector.cpp
        ConsoleVariables/FileStoreManager.hpp ConsoleVariables/FileStoreManager.cpp
        ConsoleVariables/CVar.hpp ConsoleVariables/CVar.cpp
        ConsoleVariables/CVarManager.hpp ConsoleVariables/CVarManager.cpp
        ConsoleVariables/CVarCommons.hpp ConsoleVariables/CVarCommons.cpp
        Tweaks/ITweak.hpp
        Tweaks/ITweakAutoMapper.hpp
        Tweaks/ITweakBall.hpp
        Tweaks/ITweakGame.hpp
        Tweaks/ITweakGui.hpp
        Tweaks/ITweakGuiColors.hpp
        Tweaks/ITweakGunRes.hpp
        Tweaks/ITweakParticle.hpp
        Tweaks/ITweakPlayer.hpp
        Tweaks/ITweakPlayerControl.hpp
        Tweaks/ITweakPlayerGun.hpp Tweaks/ITweakPlayerGun.cpp
        Tweaks/ITweakPlayerRes.hpp
        Tweaks/ITweakSlideShow.hpp
        Tweaks/ITweakTargeting.hpp
        IMain.hpp
        CStopwatch.hpp CStopwatch.cpp
        Streams/IOStreams.hpp Streams/IOStreams.cpp
        Streams/CMemoryStreamOut.hpp Streams/CMemoryStreamOut.cpp
        Streams/CInputStream.hpp Streams/CInputStream.cpp
        Streams/COutputStream.hpp Streams/COutputStream.cpp
        Streams/CMemoryInStream.hpp
        Streams/CZipInputStream.hpp Streams/CZipInputStream.cpp
        Streams/ContainerReaders.hpp
        Streams/CTextInStream.hpp Streams/CTextInStream.cpp
        Streams/CTextOutStream.hpp Streams/CTextOutStream.cpp
        CGameAllocator.hpp CGameAllocator.cpp
        CMemoryCardSys.hpp CMemoryCardSys.cpp
        CScannableObjectInfo.hpp CScannableObjectInfo.cpp
        CWorldSaveGameInfo.hpp CWorldSaveGameInfo.cpp
        CDependencyGroup.hpp CDependencyGroup.cpp
        CBasics.hpp CBasicsPC.cpp
        CIOWin.hpp
        CIOWinManager.hpp CIOWinManager.cpp
        CStateManager.hpp CStateManager.cpp
        CGameState.hpp CGameState.cpp
        CScriptMailbox.hpp CScriptMailbox.cpp
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
        CStringExtras.hpp CStringExtras.cpp
        CMainFlowBase.hpp CMainFlowBase.cpp
        CMFGameBase.hpp
        CInGameTweakManagerBase.hpp
        CGameDebug.hpp
        CGameHintInfo.hpp CGameHintInfo.cpp
        rstl.hpp
        GameGlobalObjects.hpp GameGlobalObjects.cpp
        GCNTypes.hpp
        CTextureCache.hpp CTextureCache.cpp
        CMayaSpline.hpp CMayaSpline.cpp
        ImGuiPlayerLoadouts.hpp ImGuiPlayerLoadouts.cpp
        ${PLAT_SRCS})

function(add_runtime_common_library name)
    add_library(${name} ${ARGN})
    target_compile_definitions(${name} PUBLIC "-DMETAFORCE_TARGET_BYTE_ORDER=__BYTE_ORDER__")
    if (WINDOWS_STORE)
        set_property(TARGET ${name} PROPERTY VS_WINRT_COMPONENT TRUE)
    endif ()
endfunction()

set(RUNTIME_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})
set(RUNTIME_LIBRARIES amuse zeus nod NESEmulator libjpeg-turbo jbus kabufuda discord-rpc logvisor OptickCore imgui aurora SDL2-static
    boo # TODO move audiodev
    ${ZLIB_LIBRARIES}
    )

add_runtime_common_library(RuntimeCommon ${RUNTIME_SOURCES_A})
target_include_directories(RuntimeCommon PUBLIC ${RUNTIME_INCLUDES})
target_link_libraries(RuntimeCommon PUBLIC ${RUNTIME_LIBRARIES})

add_runtime_common_library(RuntimeCommonB ${RUNTIME_SOURCES_B})
target_include_directories(RuntimeCommonB PUBLIC ${RUNTIME_INCLUDES})
target_link_libraries(RuntimeCommonB PUBLIC ${RUNTIME_LIBRARIES})

if (WIN32)
    configure_file(platforms/win/metaforce.rc.in "${CMAKE_CURRENT_SOURCE_DIR}/platforms/win/metaforce.rc" @ONLY)
    set(PLAT_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/platforms/win/metaforce.rc" platforms/win/metaforce.manifest)
    if (WINDOWS_STORE)
        set(UWP_ASSETS
                platforms/win/Assets/LargeTile.scale-100.png
                platforms/win/Assets/LargeTile.scale-125.png
                platforms/win/Assets/LargeTile.scale-150.png
                platforms/win/Assets/LargeTile.scale-200.png
                platforms/win/Assets/LargeTile.scale-400.png
                platforms/win/Assets/SmallTile.scale-100.png
                platforms/win/Assets/SmallTile.scale-125.png
                platforms/win/Assets/SmallTile.scale-150.png
                platforms/win/Assets/SmallTile.scale-200.png
                platforms/win/Assets/SmallTile.scale-400.png
                platforms/win/Assets/SplashScreen.scale-100.png
                platforms/win/Assets/SplashScreen.scale-125.png
                platforms/win/Assets/SplashScreen.scale-150.png
                platforms/win/Assets/SplashScreen.scale-200.png
                platforms/win/Assets/SplashScreen.scale-400.png
                platforms/win/Assets/Square44x44Logo.scale-100.png
                platforms/win/Assets/Square44x44Logo.scale-125.png
                platforms/win/Assets/Square44x44Logo.scale-150.png
                platforms/win/Assets/Square44x44Logo.scale-200.png
                platforms/win/Assets/Square44x44Logo.scale-400.png
                platforms/win/Assets/Square44x44Logo.altform-unplated_targetsize-16.png
                platforms/win/Assets/Square44x44Logo.altform-unplated_targetsize-24.png
                platforms/win/Assets/Square44x44Logo.altform-unplated_targetsize-32.png
                platforms/win/Assets/Square44x44Logo.altform-unplated_targetsize-48.png
                platforms/win/Assets/Square44x44Logo.altform-unplated_targetsize-256.png
                platforms/win/Assets/Square150x150Logo.scale-100.png
                platforms/win/Assets/Square150x150Logo.scale-125.png
                platforms/win/Assets/Square150x150Logo.scale-150.png
                platforms/win/Assets/Square150x150Logo.scale-200.png
                platforms/win/Assets/Square150x150Logo.scale-400.png
                platforms/win/Assets/metaforce.scale-100.png
                platforms/win/Assets/metaforce.scale-125.png
                platforms/win/Assets/metaforce.scale-150.png
                platforms/win/Assets/metaforce.scale-200.png
                platforms/win/Assets/metaforce.scale-400.png
                platforms/win/Assets/WideTile.scale-100.png
                platforms/win/Assets/WideTile.scale-125.png
                platforms/win/Assets/WideTile.scale-150.png
                platforms/win/Assets/WideTile.scale-200.png
                platforms/win/Assets/WideTile.scale-400.png)
        set_property(SOURCE platforms/win/Package.appxmanifest PROPERTY VS_DEPLOYMENT_CONTENT 1)
        set_property(SOURCE ${UWP_ASSETS} PROPERTY VS_DEPLOYMENT_CONTENT 1)
        set_property(SOURCE ${UWP_ASSETS} PROPERTY VS_DEPLOYMENT_LOCATION "Assets")
        list(APPEND PLAT_SRCS ${UWP_ASSETS} platforms/win/Package.appxmanifest)
    endif ()
elseif (APPLE)
    # nothing
elseif (UNIX)
    set(PLAT_LIBS rt)
endif ()

add_executable(metaforce CMain.cpp ${PLAT_SRCS}
    ImGuiConsole.hpp ImGuiConsole.cpp
    ImGuiControllerConfig.hpp ImGuiControllerConfig.cpp
    ImGuiEntitySupport.hpp ImGuiEntitySupport.cpp)
# RUNTIME_LIBRARIES repeated here for link ordering
target_link_libraries(metaforce PUBLIC RuntimeCommon RuntimeCommonB ${RUNTIME_LIBRARIES} ${PLAT_LIBS})
if (TARGET nativefiledialog)
    target_link_libraries(metaforce PRIVATE nativefiledialog)
endif()
target_compile_definitions(metaforce PUBLIC "-DMETAFORCE_TARGET_BYTE_ORDER=__BYTE_ORDER__")

if (WINDOWS_STORE)
    set_property(TARGET metaforce PROPERTY VS_WINRT_COMPONENT TRUE)
    # This should match the Package.appxmanifest
    set_property(TARGET metaforce PROPERTY VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION "10.0.14393.0")
endif ()
