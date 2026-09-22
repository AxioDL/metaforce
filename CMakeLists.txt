cmake_minimum_required(VERSION 3.25)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
            "Build type options: Debug Release RelWithDebInfo MinSizeRel" FORCE)
endif ()

include(extern/borealis/cmake/DetectVersion.cmake)
borealis_detect_version()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
project(metaforce VERSION ${BOREALIS_APP_VERSION} LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if (ENABLE_SANITIZERS)
    if (CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC" AND
            CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        add_compile_options($<$<COMPILE_LANGUAGE:C,CXX>:/fsanitize=address,undefined>)
        add_link_options(/fsanitize=address,undefined /INCREMENTAL:NO)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "ProgramDatabase")
        foreach (_lang C CXX)
            foreach (_rtc_flag /RTC1 /RTCc /RTCs /RTCu)
                string(REPLACE "${_rtc_flag}" "" CMAKE_${_lang}_FLAGS_DEBUG "${CMAKE_${_lang}_FLAGS_DEBUG}")
            endforeach ()
        endforeach ()
    elseif (CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "GNU" AND
            CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
        add_compile_options(
                $<$<COMPILE_LANGUAGE:C,CXX,OBJC,OBJCXX>:-fsanitize=address,undefined>
                $<$<COMPILE_LANGUAGE:C,CXX,OBJC,OBJCXX>:-fno-omit-frame-pointer>
        )
        add_link_options(-fsanitize=address,undefined)
    else ()
        message(FATAL_ERROR "ENABLE_SANITIZERS requires GNU-like or MSVC-like C/C++ compiler frontends")
    endif ()
    add_compile_definitions(NDEBUG_SANITIZER) # Avoids absl issue with SwissTable debug code
    message(STATUS "metaforce: Enabled sanitizers")
endif ()

set(AURORA_ENABLE_CARD ON CACHE BOOL "Enable CARD API support" FORCE)
set(AURORA_ENABLE_DVD ON CACHE BOOL "Enable DVD API support" FORCE)
set(AURORA_ENABLE_RMLUI OFF CACHE BOOL "Enable RmlUi UI support" FORCE)
set(AURORA_ENABLE_THP ON CACHE BOOL "Enable THP decoder" FORCE)
set(BOREALIS_ENABLE_DISCORD OFF CACHE BOOL "Build borealis::discord" FORCE)
set(BOREALIS_ENABLE_HTTP OFF CACHE BOOL "Build borealis::http" FORCE)
set(BOREALIS_ENABLE_NET OFF CACHE BOOL "Build borealis::net" FORCE)
set(BOREALIS_ENABLE_UPDATE OFF CACHE BOOL "Build borealis::update (requires borealis::http and nlohmann_json)" FORCE)
set(BOREALIS_ENABLE_WEBSOCKET OFF CACHE BOOL "Build borealis::ws (requires borealis::http)" FORCE)
add_subdirectory(extern/aurora EXCLUDE_FROM_ALL)
add_subdirectory(extern/borealis EXCLUDE_FROM_ALL)
add_subdirectory(extern/musyx EXCLUDE_FROM_ALL)

set(GAME_LIBS
      aurora::core aurora::gx aurora::vi aurora::pad aurora::mtx aurora::os aurora::dvd aurora::thp aurora::card aurora::main
      borealis::cli borealis::crash borealis::data borealis::disc borealis::io borealis::log borealis::presentation
      musyx ZLIB::ZLIB TracyClient fmt::fmt)

set(GAME_FILES
      src/WorldFormat/CPVSAreaSet.cpp
      src/WorldFormat/CAreaRenderOctTree.cpp
      src/WorldFormat/CCollidableOBBTree.cpp
      src/WorldFormat/CCollidableOBBTreeGroup.cpp
      src/WorldFormat/CAreaBspTree.cpp
      src/WorldFormat/CAreaOctTree_Tests.cpp
      src/WorldFormat/CCollisionSurface.cpp
      src/WorldFormat/CMetroidAreaCollider.cpp
      src/WorldFormat/CWorldLight.cpp
      src/WorldFormat/CAreaOctTree.cpp
      src/WorldFormat/CMetroidModelInstance.cpp
      src/WorldFormat/COBBTree.cpp
      src/GuiSys/CGuiFeeHelper.cpp
      src/GuiSys/CAuiEnergyBarT01.cpp
      src/GuiSys/CGuiWidget.cpp
      src/GuiSys/CGuiModel.cpp
      src/GuiSys/CGuiTableGroup.cpp
      src/GuiSys/CGuiSys.cpp
      src/GuiSys/CGuiCamera.cpp
      src/GuiSys/CGuiPane.cpp
      src/GuiSys/CGuiFactories.cpp
      src/GuiSys/CGuiObject.cpp
      src/GuiSys/CAuiMain.cpp
      src/GuiSys/CGuiGroup.cpp
      src/GuiSys/CGuiWidgetDrawParms.cpp
      src/GuiSys/CGuiTextSupport.cpp
      src/GuiSys/CGuiWidgetIdDB.cpp
      src/GuiSys/CGuiTextPane.cpp
      src/GuiSys/CGuiSliderGroup.cpp
      src/GuiSys/CGuiFrame.cpp
      src/GuiSys/CGuiHeadWidget.cpp
      src/GuiSys/CGuiCompoundWidget.cpp
      src/GuiSys/CAuiMeter.cpp
      src/GuiSys/CRepeatState.cpp
      src/GuiSys/CGuiLight.cpp
      src/GuiSys/CAuiImagePane.cpp
      src/MetaRender/CCubeRenderer.cpp
      src/MetroidPrime/CCollisionActor.cpp
      src/MetroidPrime/CMemoryCard.cpp
      src/MetroidPrime/CMFGame.cpp
      src/MetroidPrime/CArchMsgParmControllerStatus.cpp
      src/MetroidPrime/CRainSplashGenerator.cpp
      src/MetroidPrime/CMapWorld.cpp
      src/MetroidPrime/CTransitionDatabaseGame.cpp
      src/MetroidPrime/CArtifactDoll.cpp
      src/MetroidPrime/CWorldShadow.cpp
      src/MetroidPrime/CVisorFlare.cpp
      src/MetroidPrime/CGameHintInfo.cpp
      src/MetroidPrime/CSplashScreen.cpp
      src/MetroidPrime/CConsoleOutputWindow.cpp
      src/MetroidPrime/CGameCollision.cpp
      src/MetroidPrime/ScriptObjects/CScriptSpiderBallAttractionSurface.cpp
      src/MetroidPrime/ScriptObjects/CScriptProjectedShadow.cpp
      src/MetroidPrime/ScriptObjects/CScriptSpindleCamera.cpp
      src/MetroidPrime/ScriptObjects/CScriptSpawnPoint.cpp
      src/MetroidPrime/ScriptObjects/CScriptRoomAcoustics.cpp
      src/MetroidPrime/ScriptObjects/CScriptStreamedMusic.cpp
      src/MetroidPrime/ScriptObjects/CScriptBallTrigger.cpp
      src/MetroidPrime/ScriptObjects/CSustainedPlayerDamage.cpp
      src/MetroidPrime/ScriptObjects/CScriptPointOfInterest.cpp
      src/MetroidPrime/ScriptObjects/CScriptEMPulse.cpp
      src/MetroidPrime/ScriptObjects/CScriptSwitch.cpp
      src/MetroidPrime/ScriptObjects/CScriptSpiderBallWaypoint.cpp
      src/MetroidPrime/ScriptObjects/CScriptVisorFlare.cpp
      src/MetroidPrime/ScriptObjects/CFishCloud.cpp
      src/MetroidPrime/ScriptObjects/CScriptCameraHintTrigger.cpp
      src/MetroidPrime/ScriptObjects/CScriptActorRotate.cpp
      src/MetroidPrime/ScriptObjects/CScriptSound.cpp
      src/MetroidPrime/ScriptObjects/CScriptDoor.cpp
      src/MetroidPrime/ScriptObjects/CScriptCameraHint.cpp
      src/MetroidPrime/ScriptObjects/CScriptSteam.cpp
      src/MetroidPrime/ScriptObjects/CScriptVisorGoo.cpp
      src/MetroidPrime/ScriptObjects/CScriptGunTurret.cpp
      src/MetroidPrime/ScriptObjects/CHUDBillboardEffect.cpp
      src/MetroidPrime/ScriptObjects/CScriptAiJumpPoint.cpp
      src/MetroidPrime/ScriptObjects/CScriptCounter.cpp
      src/MetroidPrime/ScriptObjects/CScriptGrapplePoint.cpp
      src/MetroidPrime/ScriptObjects/CScriptGenerator.cpp
      src/MetroidPrime/ScriptObjects/CScriptMemoryRelay.cpp
      src/MetroidPrime/ScriptObjects/CRepulsor.cpp
      src/MetroidPrime/ScriptObjects/CScriptHUDMemo.cpp
      src/MetroidPrime/ScriptObjects/CScriptMazeNode.cpp
      src/MetroidPrime/ScriptObjects/CFire.cpp
      src/MetroidPrime/ScriptObjects/CScriptMidi.cpp
      src/MetroidPrime/ScriptObjects/CScriptAreaAttributes.cpp
      src/MetroidPrime/ScriptObjects/CScriptRelay.cpp
      src/MetroidPrime/ScriptObjects/CScriptPlayerActor.cpp
      src/MetroidPrime/ScriptObjects/CScriptDebris.cpp
      src/MetroidPrime/ScriptObjects/CScriptTrigger.cpp
      src/MetroidPrime/ScriptObjects/CScriptCameraWaypoint.cpp
      src/MetroidPrime/ScriptObjects/CScriptDistanceFog.cpp
      src/MetroidPrime/ScriptObjects/CScriptTargetingPoint.cpp
      src/MetroidPrime/ScriptObjects/CScriptWorldTeleporter.cpp
      src/MetroidPrime/ScriptObjects/CScriptDockAreaChange.cpp
      src/MetroidPrime/ScriptObjects/CScriptPlayerStateChange.cpp
      src/MetroidPrime/ScriptObjects/CScriptCameraShaker.cpp
      src/MetroidPrime/ScriptObjects/CScriptWaypoint.cpp
      src/MetroidPrime/ScriptObjects/CScriptRandomRelay.cpp
      src/MetroidPrime/ScriptObjects/CScriptControllerAction.cpp
      src/MetroidPrime/ScriptObjects/CEnergyBall.cpp
      src/MetroidPrime/ScriptObjects/CScriptPickup.cpp
      src/MetroidPrime/ScriptObjects/CScriptBeam.cpp
      src/MetroidPrime/ScriptObjects/CScriptDebugCameraWaypoint.cpp
      src/MetroidPrime/ScriptObjects/CScriptCameraBlurKeyframe.cpp
      src/MetroidPrime/ScriptObjects/CScriptTimer.cpp
      src/MetroidPrime/ScriptObjects/CScriptDock.cpp
      src/MetroidPrime/ScriptObjects/CScriptActor.cpp
      src/MetroidPrime/ScriptObjects/CScriptPickupGenerator.cpp
      src/MetroidPrime/ScriptObjects/CScriptActorKeyframe.cpp
      src/MetroidPrime/ScriptObjects/CScriptColorModulate.cpp
      src/MetroidPrime/ScriptObjects/CScriptCameraPitchVolume.cpp
      src/MetroidPrime/ScriptObjects/CScriptEffect.cpp
      src/MetroidPrime/ScriptObjects/CScriptPlayerHint.cpp
      src/MetroidPrime/ScriptObjects/CScriptRipple.cpp
      src/MetroidPrime/ScriptObjects/CScriptWater.cpp
      src/MetroidPrime/ScriptObjects/CScriptPlatform.cpp
      src/MetroidPrime/ScriptObjects/CScriptCoverPoint.cpp
      src/MetroidPrime/ScriptObjects/CScriptSpecialFunction.cpp
      src/MetroidPrime/ScriptObjects/CScriptDamageableTrigger.cpp
      src/MetroidPrime/ScriptObjects/CScriptCameraFilterKeyframe.cpp
      src/MetroidPrime/TGameTypes.cpp
      src/MetroidPrime/CArchMsgParmReal32.cpp
      src/MetroidPrime/CArchMsgParmInt32.cpp
      src/MetroidPrime/CExplosion.cpp
      src/MetroidPrime/CArchMsgParmUserInput.cpp
      src/MetroidPrime/CInputGenerator.cpp
      src/MetroidPrime/CInGameTweakManager.cpp
      src/MetroidPrime/CIkChain.cpp
      src/MetroidPrime/CFluidPlane.cpp
      src/MetroidPrime/CBallFilter.cpp
      src/MetroidPrime/CEulerAngles.cpp
      src/MetroidPrime/CCredits.cpp
      src/MetroidPrime/CHealthInfo.cpp
      src/MetroidPrime/CPreFrontEnd.cpp
      src/MetroidPrime/BodyState/CBSProjectileAttack.cpp
      src/MetroidPrime/BodyState/CBSGroundHit.cpp
      src/MetroidPrime/BodyState/CABSIdle.cpp
      src/MetroidPrime/BodyState/CBSLieOnGround.cpp
      src/MetroidPrime/BodyState/CBSLoopAttack.cpp
      src/MetroidPrime/BodyState/CBSHurled.cpp
      src/MetroidPrime/BodyState/CBSTurn.cpp
      src/MetroidPrime/BodyState/CBSKnockBack.cpp
      src/MetroidPrime/BodyState/CABSReaction.cpp
      src/MetroidPrime/BodyState/CBodyController.cpp
      src/MetroidPrime/BodyState/CBodyStateInfo.cpp
      src/MetroidPrime/BodyState/CBodyStateCmdMgr.cpp
      src/MetroidPrime/BodyState/CBSGetup.cpp
      src/MetroidPrime/BodyState/CBSDie.cpp
      src/MetroidPrime/BodyState/CBSWallHang.cpp
      src/MetroidPrime/BodyState/CBSFall.cpp
      src/MetroidPrime/BodyState/CBSGenerate.cpp
      src/MetroidPrime/BodyState/CBSCover.cpp
      src/MetroidPrime/BodyState/CBSStep.cpp
      src/MetroidPrime/BodyState/CABSFlinch.cpp
      src/MetroidPrime/BodyState/CBSScripted.cpp
      src/MetroidPrime/BodyState/CBSLoopReaction.cpp
      src/MetroidPrime/BodyState/CABSAim.cpp
      src/MetroidPrime/BodyState/CBSSlide.cpp
      src/MetroidPrime/BodyState/CBSJump.cpp
      src/MetroidPrime/BodyState/CBSTaunt.cpp
      src/MetroidPrime/BodyState/CBSLocomotion.cpp
      src/MetroidPrime/BodyState/CBSAttack.cpp
      src/MetroidPrime/CCollisionActorManager.cpp
      src/MetroidPrime/CFlameWarp.cpp
      src/MetroidPrime/CMapUniverse.cpp
      src/MetroidPrime/ScriptLoader.cpp
      src/MetroidPrime/Clamp_int.cpp
      src/MetroidPrime/CFluidPlaneDoor.cpp
      src/MetroidPrime/CSaveGameScreen.cpp
      src/MetroidPrime/CGameCubeDoll.cpp
      src/MetroidPrime/CNESEmulator.cpp
      src/MetroidPrime/PathFinding/CPathFindRegion.cpp
      src/MetroidPrime/PathFinding/CPathFindArea.cpp
      src/MetroidPrime/PathFinding/CPathFindSpline.cpp
      src/MetroidPrime/PathFinding/CPathFindSearch.cpp
      src/MetroidPrime/CActorLights.cpp
      src/MetroidPrime/CObjectList.cpp
      src/MetroidPrime/CIOWinManager.cpp
      src/MetroidPrime/CAutoMapper.cpp
      src/MetroidPrime/CEntity.cpp
      src/MetroidPrime/GameObjectLists.cpp
      src/MetroidPrime/CScriptMailbox.cpp
      src/MetroidPrime/CStateManager.cpp
      src/MetroidPrime/CParticleDatabase.cpp
      src/MetroidPrime/CRumbleManager.cpp
      src/MetroidPrime/CSlideShow.cpp
      src/MetroidPrime/CActorModelParticles.cpp
      src/MetroidPrime/UserNames.cpp
      src/MetroidPrime/RumbleFxTable.cpp
      src/MetroidPrime/CMemoryDrawEnum.cpp
      src/MetroidPrime/CFluidPlaneManager.cpp
      src/MetroidPrime/CActor.cpp
      src/MetroidPrime/Weapons/CPhazonBeam.cpp
      src/MetroidPrime/Weapons/CIceImpact.cpp
      src/MetroidPrime/Weapons/CIceProjectile.cpp
      src/MetroidPrime/Weapons/CGunWeapon.cpp
      src/MetroidPrime/Weapons/CWeapon.cpp
      src/MetroidPrime/Weapons/CPlasmaBeam.cpp
      src/MetroidPrime/Weapons/CBomb.cpp
      src/MetroidPrime/Weapons/CWaveBuster.cpp
      src/MetroidPrime/Weapons/CPlasmaProjectile.cpp
      src/MetroidPrime/Weapons/WeaponTypes.cpp
      src/MetroidPrime/Weapons/CElectricBeamProjectile.cpp
      src/MetroidPrime/Weapons/CEnergyProjectile.cpp
      src/MetroidPrime/Weapons/CProjectileInfo.cpp
      src/MetroidPrime/Weapons/CGameProjectile.cpp
      src/MetroidPrime/Weapons/CBeamProjectile.cpp
      src/MetroidPrime/Weapons/CAuxWeapon.cpp
      src/MetroidPrime/Weapons/CTargetableProjectile.cpp
      src/MetroidPrime/Weapons/CPowerBomb.cpp
      src/MetroidPrime/Weapons/CFlameThrower.cpp
      src/MetroidPrime/Weapons/CIceBeam.cpp
      src/MetroidPrime/Weapons/CShockWave.cpp
      src/MetroidPrime/Weapons/CWaveBeam.cpp
      src/MetroidPrime/Weapons/GunController/CGunMotion.cpp
      src/MetroidPrime/Weapons/GunController/CGSComboFire.cpp
      src/MetroidPrime/Weapons/GunController/CGSFidget.cpp
      src/MetroidPrime/Weapons/GunController/CGunController.cpp
      src/MetroidPrime/Weapons/GunController/CGSFreeLook.cpp
      src/MetroidPrime/Weapons/CPowerBeam.cpp
      src/MetroidPrime/Weapons/CNewFlameThrower.cpp
      src/MetroidPrime/CFluidPlaneRender.cpp
      src/MetroidPrime/CErrorOutputWindow.cpp
      src/MetroidPrime/CProjectedShadow.cpp
      src/MetroidPrime/CSteeringBehaviors.cpp
      src/MetroidPrime/CPauseScreenBlur.cpp
      src/MetroidPrime/CRippleManager.cpp
      src/MetroidPrime/Cameras/CInterpolationCamera.cpp
      src/MetroidPrime/Cameras/CCameraManager.cpp
      src/MetroidPrime/Cameras/CCameraFilter.cpp
      src/MetroidPrime/Cameras/CBallCameraFailsafeState.cpp
      src/MetroidPrime/Cameras/CCinematicCamera.cpp
      src/MetroidPrime/Cameras/CCameraShakeData.cpp
      src/MetroidPrime/Cameras/CBallCamera.cpp
      src/MetroidPrime/Cameras/CFirstPersonCamera.cpp
      src/MetroidPrime/Cameras/CPathCamera.cpp
      src/MetroidPrime/Cameras/CGameCamera.cpp
      src/MetroidPrime/CParticleGenInfo.cpp
      src/MetroidPrime/CAxisAngle.cpp
      src/MetroidPrime/CPauseScreenFrame.cpp
      src/MetroidPrime/CRagDoll.cpp
      src/MetroidPrime/CFluidUVMotion.cpp
      src/MetroidPrime/CMatrix3f_Ext.cpp
      src/MetroidPrime/CMessageScreen.cpp
      src/MetroidPrime/CScriptLayerManager.cpp
      src/MetroidPrime/CPhysicsActor.cpp
      src/MetroidPrime/CWorldTransManager.cpp
      src/MetroidPrime/CAudioStateWin.cpp
      src/MetroidPrime/CActorParameters.cpp
      src/MetroidPrime/CPhysicsState.cpp
      src/MetroidPrime/CInGameGuiManager.cpp
      src/MetroidPrime/CIOWin.cpp
      src/MetroidPrime/CSamusDoll.cpp
      src/MetroidPrime/CArchMsgParmNull.cpp
      src/MetroidPrime/CTargetReticles.cpp
      src/MetroidPrime/CSimpleShadow.cpp
      src/MetroidPrime/CArchMsgParmInt32Int32VoidPtr.cpp
      src/MetroidPrime/CSortedLists.cpp
      src/MetroidPrime/CEnvFxManager.cpp
      src/MetroidPrime/Factories/CStateMachineFactory.cpp
      src/MetroidPrime/Factories/CScannableObjectInfo.cpp
      src/MetroidPrime/Factories/CCharacterFactory.cpp
      src/MetroidPrime/Factories/CAssetFactory.cpp
      src/MetroidPrime/CMemoryCardDriver.cpp
      src/MetroidPrime/CGroundMovement.cpp
      src/MetroidPrime/Decode.cpp
      src/MetroidPrime/CAnimRes.cpp
      src/MetroidPrime/CParticleGenInfoGeneric.cpp
      src/MetroidPrime/CRipple.cpp
      src/MetroidPrime/CGameArea.cpp
      src/MetroidPrime/CPauseScreen.cpp
      src/MetroidPrime/CMappableObject.cpp
      src/MetroidPrime/Tweaks/CTweakPlayer.cpp
      src/MetroidPrime/Tweaks/CTweakPlayerGun.cpp
      src/MetroidPrime/Tweaks/CTweakParticle.cpp
      src/MetroidPrime/Tweaks/CTweakGunRes.cpp
      src/MetroidPrime/Tweaks/CTweakTargeting.cpp
      src/MetroidPrime/Tweaks/CTweakSlideShow.cpp
      src/MetroidPrime/Tweaks/CTweakPlayerControl.cpp
      src/MetroidPrime/Tweaks/CTweakGui.cpp
      src/MetroidPrime/Tweaks/CTweakAutoMapper.cpp
      src/MetroidPrime/Tweaks/CTweakPlayerRes.cpp
      src/MetroidPrime/Tweaks/CTweakGuiColors.cpp
      src/MetroidPrime/Tweaks/CTweakBall.cpp
      src/MetroidPrime/Tweaks/CTweakGame.cpp
      src/MetroidPrime/Tweaks/CTweaks.cpp
      src/MetroidPrime/CGameLight.cpp
      src/MetroidPrime/CControlMapper.cpp
      src/MetroidPrime/CAnimData.cpp
      src/MetroidPrime/CLogBookScreen.cpp
      src/MetroidPrime/CDamageInfo.cpp
      src/MetroidPrime/CDamageVulnerability.cpp
      src/MetroidPrime/CWorld.cpp
      src/MetroidPrime/CInventoryScreen.cpp
      src/MetroidPrime/CWeaponMgr.cpp
      src/MetroidPrime/CWorldSaveGameInfo.cpp
      src/MetroidPrime/CMainFlow.cpp
      src/MetroidPrime/CMapWorldInfo.cpp
      src/MetroidPrime/CAnimationDatabaseGame.cpp
      src/MetroidPrime/main.cpp
      src/MetroidPrime/CFrontEndUI.cpp
      src/MetroidPrime/CModelData.cpp
      # src/MetroidPrime/TypesMatch.cpp
      src/MetroidPrime/CGBASupport.cpp
      src/MetroidPrime/CMapArea.cpp
      src/MetroidPrime/CAABoxFilter.cpp
      src/MetroidPrime/CBoneTracking.cpp
      src/MetroidPrime/CFluidPlaneCPU.cpp
      src/MetroidPrime/Enemies/CFireFlea.cpp
      src/MetroidPrime/Enemies/SPositionHistory.cpp
      src/MetroidPrime/Enemies/CFlickerBat.cpp
      src/MetroidPrime/Enemies/CBabygoth.cpp
      src/MetroidPrime/Enemies/CJellyZap.cpp
      src/MetroidPrime/Enemies/CIceSheegoth.cpp
      src/MetroidPrime/Enemies/CMetroidPrimeStage2.cpp
      src/MetroidPrime/Enemies/CFlaahgra.cpp
      src/MetroidPrime/Enemies/CMagdolite.cpp
      src/MetroidPrime/Enemies/CWarWasp.cpp
      src/MetroidPrime/Enemies/CPoisonProjectile.cpp
      src/MetroidPrime/Enemies/CMetroidPrime.cpp
      src/MetroidPrime/Enemies/CMetroid.cpp
      src/MetroidPrime/Enemies/CElitePirate.cpp
      src/MetroidPrime/Enemies/CSnakeWeedSwarm.cpp
      src/MetroidPrime/Enemies/CBurstFire.cpp
      src/MetroidPrime/Enemies/CAtomicAlpha.cpp
      src/MetroidPrime/Enemies/CThardus.cpp
      src/MetroidPrime/Enemies/CMetroidPrimeRelay.cpp
      src/MetroidPrime/Enemies/PatternedCastTo.cpp
      src/MetroidPrime/Enemies/CPuffer.cpp
      src/MetroidPrime/Enemies/CParasite.cpp
      src/MetroidPrime/Enemies/CDrone.cpp
      src/MetroidPrime/Enemies/CSeedling.cpp
      src/MetroidPrime/Enemies/CFlaahgraTentacle.cpp
      src/MetroidPrime/Enemies/CTeamAiMgr.cpp
      src/MetroidPrime/Enemies/CPhazonHealingNodule.cpp
      src/MetroidPrime/Enemies/CScriptContraption.cpp
      src/MetroidPrime/Enemies/CPatternedInfo.cpp
      src/MetroidPrime/Enemies/CScriptPhazonPool.cpp
      src/MetroidPrime/Enemies/CRipper.cpp
      src/MetroidPrime/Enemies/CThardusRockProjectile.cpp
      src/MetroidPrime/Enemies/CBeetle.cpp
      src/MetroidPrime/Enemies/CEyeBall.cpp
      src/MetroidPrime/Enemies/CBurrower.cpp
      src/MetroidPrime/Enemies/CBloodFlower.cpp
      src/MetroidPrime/Enemies/CBouncyGrenade.cpp
      src/MetroidPrime/Enemies/CKnockBackMgr.cpp
      src/MetroidPrime/Enemies/CNewIntroBoss.cpp
      src/MetroidPrime/Enemies/CAi.cpp
      src/MetroidPrime/Enemies/CWallWalker.cpp
      src/MetroidPrime/Enemies/CChozoGhost.cpp
      src/MetroidPrime/Enemies/CPuddleToadGamma.cpp
      src/MetroidPrime/Enemies/CMetroidBeta.cpp
      src/MetroidPrime/Enemies/CFlyingPirate.cpp
      src/MetroidPrime/Enemies/CAmbientAI.cpp
      src/MetroidPrime/Enemies/CFlaahgraPlants.cpp
      src/MetroidPrime/Enemies/CPuddleSpore.cpp
      src/MetroidPrime/Enemies/CSpankWeed.cpp
      src/MetroidPrime/Enemies/CGrenadeLauncher.cpp
      src/MetroidPrime/Enemies/CMetaree.cpp
      src/MetroidPrime/Enemies/CRidley.cpp
      src/MetroidPrime/Enemies/CWallCrawlerSwarm.cpp
      src/MetroidPrime/Enemies/CPatternedAiFunctions.cpp
      src/MetroidPrime/Enemies/COmegaPirate.cpp
      src/MetroidPrime/Enemies/CStateMachine.cpp
      src/MetroidPrime/Enemies/CAtomicBeta.cpp
      src/MetroidPrime/Enemies/CPatterned.cpp
      src/MetroidPrime/Enemies/CSpacePirate.cpp
      src/MetroidPrime/Enemies/CRipperControlledPlatform.cpp
      src/MetroidPrime/Enemies/CTryclops.cpp
      src/MetroidPrime/Player/CPlayerDynamics.cpp
      src/MetroidPrime/Player/CMorphBallShadow.cpp
      src/MetroidPrime/Player/CPlayerEnergyDrain.cpp
      src/MetroidPrime/Player/CGrappleArm.cpp
      src/MetroidPrime/Player/CFaceplateDecoration.cpp
      src/MetroidPrime/Player/CScanDisplay.cpp
      src/MetroidPrime/Player/CMorphBall.cpp
      src/MetroidPrime/Player/CPlayer.cpp
      src/MetroidPrime/Player/CPlayerGun.cpp
      src/MetroidPrime/Player/CPlayerOrbit.cpp
      src/MetroidPrime/Player/CPlayerCameraBob.cpp
      src/MetroidPrime/Player/CFidget.cpp
      src/MetroidPrime/Player/CPlayerVisor.cpp
      src/MetroidPrime/Player/CPlayerStuckTracker.cpp
      src/MetroidPrime/Player/CGameOptions.cpp
      src/MetroidPrime/Player/CSamusFaceReflection.cpp
      src/MetroidPrime/Player/CGameState.cpp
      src/MetroidPrime/Player/CPlayerState.cpp
      src/MetroidPrime/Player/CStaticInterference.cpp
      src/MetroidPrime/CEffect.cpp
      src/MetroidPrime/TCastTo.cpp
      src/MetroidPrime/HUD/CHUDMemoParms.cpp
      src/MetroidPrime/HUD/CHudDecoInterface.cpp
      src/MetroidPrime/HUD/CHudEnergyInterface.cpp
      src/MetroidPrime/HUD/CHudVisorBeamMenu.cpp
      src/MetroidPrime/HUD/CHudMissileInterface.cpp
      src/MetroidPrime/HUD/CHudBallInterface.cpp
      src/MetroidPrime/HUD/CHudFreeLookInterface.cpp
      src/MetroidPrime/HUD/CHudRadarInterface.cpp
      src/MetroidPrime/HUD/CHudThreatInterface.cpp
      src/MetroidPrime/HUD/CSamusHud.cpp
      src/MetroidPrime/HUD/CHudHelmetInterface.cpp
      src/MetroidPrime/CDecalManager.cpp
      src/Weapons/CProjectileWeapon.cpp
      src/Weapons/IWeaponRenderer.cpp
      src/Weapons/CDecal.cpp
      src/Weapons/CProjectileWeaponDataFactory.cpp
      src/Weapons/CDecalDescription.cpp
      src/Weapons/CCollisionResponseData.cpp
      src/Weapons/CDecalDataFactory.cpp
      src/Weapons/CWeaponDescription.cpp
      src/Kyoto/Basics/CSWDataDolphin.cpp
      src/Kyoto/Basics/CBasics.cpp
      src/Kyoto/Basics/CBasicsDolphin.cpp
      src/Kyoto/Basics/COsContextDolphin.cpp
      src/Kyoto/Basics/RAssertDolphin.cpp
      src/Kyoto/Basics/CStopwatch.cpp
      src/Kyoto/CSimplePool.cpp
      src/Kyoto/IObj.cpp
      src/Kyoto/Alloc/CCallStackDolphin.cpp
      # src/Kyoto/Alloc/CCircularBuffer.cpp
      # src/Kyoto/Alloc/CMediumAllocPool.cpp
      # src/Kyoto/Alloc/CGameAllocator.cpp
      # src/Kyoto/Alloc/CMemory.cpp
      # src/Kyoto/Alloc/CSmallAllocPool.cpp
      # src/Kyoto/Alloc/IAllocator.cpp
      # src/Kyoto/CARAMManager.cpp
      src/Kyoto/CDependencyGroup.cpp
      src/Kyoto/Input/CRumbleVoice.cpp
      src/Kyoto/Input/CFinalInput.cpp
      src/Kyoto/Input/RumbleAdsr.cpp
      src/Kyoto/Input/CRumbleGenerator.cpp
      src/Kyoto/Input/DolphinIController.cpp
      src/Kyoto/Input/CDolphinController.cpp
      src/Kyoto/CDvdRequest.cpp
      src/Kyoto/Streams/CZipOutputStream.cpp
      src/Kyoto/Streams/CInputStream.cpp
      src/Kyoto/Streams/COutputStream.cpp
      src/Kyoto/Streams/CZipSupport.cpp
      src/Kyoto/Streams/CZipInputStream.cpp
      src/Kyoto/Streams/CMemoryStreamOut.cpp
      src/Kyoto/Streams/CMemoryInStream.cpp
      src/Kyoto/PVS/CPVSVisOctree.cpp
      src/Kyoto/PVS/CPVSVisSet.cpp
      src/Kyoto/CFrameDelayedKiller.cpp
      src/Kyoto/CRandom16.cpp
      src/Kyoto/Animation/CFBStreamedAnimReader.cpp
      src/Kyoto/Animation/CAnimTreeNode.cpp
      src/Kyoto/Animation/CMetaTransPhaseTrans.cpp
      src/Kyoto/Animation/CPASAnimState.cpp
      src/Kyoto/Animation/CInt32POINode.cpp
      src/Kyoto/Animation/CVertexMorphEffect.cpp
      src/Kyoto/Animation/CAnimTreeTimeScale.cpp
      src/Kyoto/Animation/CCharacterInfo.cpp
      src/Kyoto/Animation/CAnimTreeTweenBase.cpp
      src/Kyoto/Animation/CSegStatementSet.cpp
      src/Kyoto/Animation/CAnimTreeBlend.cpp
      src/Kyoto/Animation/CTimeScaleFunctions.cpp
      src/Kyoto/Animation/CMetaAnimPhaseBlend.cpp
      src/Kyoto/Animation/CAnimTreeSequence.cpp
      src/Kyoto/Animation/CHierarchyPoseBuilder.cpp
      src/Kyoto/Animation/CTransitionManager.cpp
      src/Kyoto/Animation/CMetaAnimSequence.cpp
      src/Kyoto/Animation/CSequenceHelper.cpp
      src/Kyoto/Animation/CTreeUtils.cpp
      src/Kyoto/Animation/CAnimCharacterSet.cpp
      src/Kyoto/Animation/CMetaAnimPlay.cpp
      src/Kyoto/Animation/CBoolPOINode.cpp
      src/Kyoto/Animation/IAnimReader.cpp
      src/Kyoto/Animation/IMetaAnim.cpp
      src/Kyoto/Animation/CAdditiveAnimPlayback.cpp
      src/Kyoto/Animation/CSegIdList.cpp
      src/Kyoto/Animation/CAnimTreeLoopIn.cpp
      src/Kyoto/Animation/CAnimPOIData.cpp
      src/Kyoto/Animation/DolphinCVirtualBone.cpp
      src/Kyoto/Animation/CAdvancementDeltas.cpp
      src/Kyoto/Animation/CSkinnedModelWithAvgNormals.cpp
      src/Kyoto/Animation/DolphinCSkinnedModel.cpp
      src/Kyoto/Animation/CAnimPerSegmentData.cpp
      src/Kyoto/Animation/CAnimSourceReader.cpp
      src/Kyoto/Animation/CAnimTreeTransition.cpp
      src/Kyoto/Animation/CAnimation.cpp
      src/Kyoto/Animation/CPASParmInfo.cpp
      src/Kyoto/Animation/CAnimTreeDoubleChild.cpp
      src/Kyoto/Animation/CAnimationManager.cpp
      src/Kyoto/Animation/CMetaAnimFactory.cpp
      src/Kyoto/Animation/CTransition.cpp
      src/Kyoto/Animation/CAnimSource.cpp
      src/Kyoto/Animation/CPOINode.cpp
      src/Kyoto/Animation/CCharacterSet.cpp
      src/Kyoto/Animation/CAnimTreeAnimReaderContainer.cpp
      src/Kyoto/Animation/CAnimationSet.cpp
      src/Kyoto/Animation/CPASAnimParm.cpp
      src/Kyoto/Animation/CPoseAsTransformsVariableSize.cpp
      src/Kyoto/Animation/CCharAnimTime.cpp
      src/Kyoto/Animation/CMetaTransFactory.cpp
      src/Kyoto/Animation/CPASAnimInfo.cpp
      src/Kyoto/Animation/CAnimTreeSingleChild.cpp
      src/Kyoto/Animation/CAllFormatsAnimSource.cpp
      src/Kyoto/Animation/CTimeRemainderAndFraction.cpp
      src/Kyoto/Animation/CHalfTransition.cpp
      src/Kyoto/Animation/CPASDatabase.cpp
      src/Kyoto/Animation/CCharLayoutInfo.cpp
      src/Kyoto/Animation/CCharAnimMemoryMetrics.cpp
      src/Kyoto/Animation/CPrimitive.cpp
      src/Kyoto/Animation/CMetaTransTrans.cpp
      src/Kyoto/Animation/CMetaAnimRandom.cpp
      src/Kyoto/Animation/CFBStreamedCompression.cpp
      src/Kyoto/Animation/CAnimMathUtils.cpp
      src/Kyoto/Animation/DolphinCSkinRules.cpp
      src/Kyoto/Animation/CAnimTreeContinuousPhaseBlend.cpp
      src/Kyoto/Animation/CMetaTransSnap.cpp
      src/Kyoto/Animation/CMetaTransMetaAnim.cpp
      src/Kyoto/Animation/CMetaAnimBlend.cpp
      src/Kyoto/Animation/CParticlePOINode.cpp
      src/Kyoto/Animation/CAnimSourceReaderBase.cpp
      src/Kyoto/CCrc32.cpp
      src/Kyoto/CDvdRequestManager.cpp
      src/Kyoto/Particles/CParticleDataFactory.cpp
      src/Kyoto/Particles/CSpawnSystemKeyframeData.cpp
      src/Kyoto/Particles/CParticleGlobals.cpp
      src/Kyoto/Particles/CParticleSwooshDataFactory.cpp
      src/Kyoto/Particles/CGenDescription.cpp
      src/Kyoto/Particles/CModVectorElement.cpp
      src/Kyoto/Particles/CParticleElectric.cpp
      src/Kyoto/Particles/CWarp.cpp
      src/Kyoto/Particles/CEffectComponent.cpp
      src/Kyoto/Particles/CElementGen.cpp
      src/Kyoto/Particles/CColorElement.cpp
      src/Kyoto/Particles/CParticleData.cpp
      src/Kyoto/Particles/CParticleElectricDataFactory.cpp
      src/Kyoto/Particles/CSwooshDescription.cpp
      src/Kyoto/Particles/CEmitterElement.cpp
      src/Kyoto/Particles/CRealElement.cpp
      src/Kyoto/Particles/CElectricDescription.cpp
      src/Kyoto/Particles/CIntElement.cpp
      src/Kyoto/Particles/CUVElement.cpp
      src/Kyoto/Particles/CParticleSwoosh.cpp
      src/Kyoto/Particles/CParticleGen.cpp
      src/Kyoto/Particles/CVectorElement.cpp
      src/Kyoto/Math/CVector3f.cpp
      src/Kyoto/Math/CQuaternion.cpp
      src/Kyoto/Math/CVector3d.cpp
      src/Kyoto/Math/CMatrix3f.cpp
      src/Kyoto/Math/CAABox.cpp
      src/Kyoto/Math/CVector2f.cpp
      src/Kyoto/Math/CMatrix4f.cpp
      src/Kyoto/Math/CUnitVector3f.cpp
      src/Kyoto/Math/RMathUtils.cpp
      src/Kyoto/Math/CloseEnough.cpp
      src/Kyoto/Math/CNUQuaternion.cpp
      src/Kyoto/Math/CSphere.cpp
      src/Kyoto/Math/CPlane.cpp
      src/Kyoto/Math/CFrustumPlanes.cpp
      src/Kyoto/Math/CVector2i.cpp
      src/Kyoto/Math/CVector3i.cpp
      src/Kyoto/Math/CTransform4f.cpp
      src/Kyoto/DolphinCMemoryCardSys.cpp
      src/Kyoto/Audio/CStaticAudioPlayer.cpp
      src/Kyoto/Audio/CStreamAudioManager.cpp
      src/Kyoto/Audio/CDSPStream.cpp
      src/Kyoto/Audio/DolphinCAudioGroupSet.cpp
      src/Kyoto/Audio/g721.cpp
      src/Kyoto/Audio/CSfxHandle.cpp
      src/Kyoto/Audio/DolphinCAudioSys.cpp
      src/Kyoto/Audio/CDSPStreamManager.cpp
      src/Kyoto/Audio/CSfxManager.cpp
      src/Kyoto/Audio/CMidiManager.cpp
      src/Kyoto/CFactoryStore.cpp
      src/Kyoto/CObjectReference.cpp
      src/Kyoto/CResLoader.cpp
      src/Kyoto/CTimeProvider.cpp
      src/Kyoto/Text/CLineExtraSpaceInstruction.cpp
      src/Kyoto/Text/CLineInstruction.cpp
      src/Kyoto/Text/CRasterFont.cpp
      src/Kyoto/Text/CColorInstruction.cpp
      src/Kyoto/Text/CTextRenderBuffer.cpp
      src/Kyoto/Text/CBlockInstruction.cpp
      src/Kyoto/Text/CRemoveColorOverrideInstruction.cpp
      src/Kyoto/Text/CStringTable.cpp
      src/Kyoto/Text/CFontRenderState.cpp
      src/Kyoto/Text/CColorOverrideInstruction.cpp
      src/Kyoto/Text/CLineSpacingInstruction.cpp
      src/Kyoto/Text/CTextInstruction.cpp
      src/Kyoto/Text/CWordBreakTables.cpp
      src/Kyoto/Text/CFontInstruction.cpp
      src/Kyoto/Text/CDrawStringOptions.cpp
      src/Kyoto/Text/CPushStateInstruction.cpp
      src/Kyoto/Text/CImageInstruction.cpp
      src/Kyoto/Text/CPopStateInstruction.cpp
      src/Kyoto/Text/CTextExecuteBuffer.cpp
      src/Kyoto/Text/CWordInstruction.cpp
      src/Kyoto/Text/CSaveableState.cpp
      src/Kyoto/Text/CFont.cpp
      src/Kyoto/Text/CTextParser.cpp
      src/Kyoto/Text/CFontImageDef.cpp
      src/Kyoto/CToken.cpp
      src/Kyoto/CPakFile.cpp
      src/Kyoto/CResFactory.cpp
      src/Kyoto/CFactoryMgr.cpp
      src/Kyoto/Graphics/DolphinCTexture.cpp
      src/Kyoto/Graphics/CCubeSurface.cpp
      src/Kyoto/Graphics/DolphinCGraphics.cpp
      src/Kyoto/Graphics/DolphinCModel.cpp
      src/Kyoto/Graphics/CTevCombiners.cpp
      src/Kyoto/Graphics/CColor.cpp
      src/Kyoto/Graphics/CCubeModel.cpp
      src/Kyoto/Graphics/CLight.cpp
      src/Kyoto/Graphics/CCubeMaterial.cpp
      src/Kyoto/Graphics/CCubeMoviePlayer.cpp
      src/Kyoto/Graphics/DolphinCPalette.cpp
      src/Kyoto/Graphics/CGX.cpp
      src/Kyoto/Graphics/DolphinCColor.cpp
      # src/Kyoto/CARAMToken.cpp
      src/Kyoto/DolphinCDvdFile.cpp
      # src/NESemu/modwrapper.cpp
      # src/NESemu/emusound.cpp
      # src/NESemu/emu.cpp
      # src/NESemu/ksNesAudio.cpp
      src/Collision/COBBox.cpp
      src/Collision/CollisionUtil.cpp
      src/Collision/CCollidableSphere.cpp
      src/Collision/CMRay.cpp
      src/Collision/CCollisionPrimitive.cpp
      src/Collision/CMaterialFilter.cpp
      src/Collision/CMaterialList.cpp
      src/Collision/CCollidableAABox.cpp
      src/Collision/CCollisionInfo.cpp
      src/Collision/InternalColliders.cpp
      src/Collision/CCollidableCollisionSurface.cpp
      extern/rstl/src/rstl/rstl_map.cpp
      extern/rstl/src/rstl/rstl_allocator.cpp
      extern/rstl/src/rstl/RstlExtras.cpp
      extern/rstl/src/rstl/rstl_strings.cpp
      src/Metaforce/ARAMToken.cpp
      src/Metaforce/Alloc.cpp
      src/Metaforce/CModelSectionReader.cpp
      src/Metaforce/MapData.cpp
      src/Metaforce/PathData.cpp
      src/Metaforce/Runtime.cpp
      src/Metaforce/Stubs.cpp
)

if (ANDROID)
    add_library(metaforce SHARED ${GAME_FILES})
else ()
    add_executable(metaforce ${GAME_FILES})
endif ()
target_compile_definitions(metaforce PRIVATE TARGET_PC=1 VERSION=0 NONMATCHING=1)
target_include_directories(metaforce PRIVATE include extern/rstl/include)
target_link_libraries(metaforce PRIVATE ${GAME_LIBS})
borealis_configure_android_application(metaforce)

if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
    target_compile_options(metaforce PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-Werror=delete-incomplete>
        -Wno-switch
        -Wno-deprecated-enum-compare
        -Wno-deprecated-enum-compare-conditional
        -Wno-deprecated-enum-enum-conversion
        -Wno-trigraphs
    )
elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    target_compile_options(metaforce PRIVATE
        /we4150 # deletion of pointer to incomplete type 'type'; no destructor called
    )
endif ()

# Require a complete type for member pointers (MSVC ABI footgun)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if (MSVC)
        target_compile_options(metaforce PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:/clang:-fcomplete-member-pointers>")
    else()
        target_compile_options(metaforce PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-fcomplete-member-pointers>")
    endif()
elseif (MSVC)
    target_compile_options(metaforce PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:/we5243>")
endif()

# Packaging logic
function(get_target_output_name target result_var)
    get_target_property(output_name ${target} OUTPUT_NAME)
    if (NOT output_name)
        set(${result_var} "${target}" PARENT_SCOPE)
    else ()
        set(${result_var} "${output_name}" PARENT_SCOPE)
    endif ()
endfunction()
function(get_target_prefix target result_var)
    set(${result_var} "" PARENT_SCOPE)
    if (APPLE)
        # Have to recreate some bundle logic here, since CMake can't tell us
        get_target_property(is_bundle ${target} MACOSX_BUNDLE)
        if (is_bundle)
            get_target_output_name(${target} output_name)
            if (CMAKE_SYSTEM_NAME STREQUAL Darwin)
                set(${result_var} "${output_name}.app/Contents/MacOS/" PARENT_SCOPE)
            else ()
                set(${result_var} "${output_name}.app/" PARENT_SCOPE)
            endif ()
        endif ()
    endif ()
endfunction()
list(APPEND BINARY_TARGETS metaforce)
set(EXTRA_TARGETS "")
if (TARGET crashpad_handler)
    list(APPEND EXTRA_TARGETS crashpad_handler)
endif ()
  install(TARGETS ${BINARY_TARGETS} ${EXTRA_TARGETS} DESTINATION ${CMAKE_INSTALL_PREFIX})
aurora_install_runtime_dlls(metaforce ${CMAKE_INSTALL_PREFIX})
if (NOT APPLE)
      install(DIRECTORY ${CMAKE_SOURCE_DIR}/res DESTINATION ${CMAKE_INSTALL_PREFIX})
endif ()
if (CMAKE_BUILD_TYPE STREQUAL Debug OR CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
    set(DEBUG_FILES_LIST "")
    foreach (target IN LISTS BINARY_TARGETS EXTRA_TARGETS)
        get_target_output_name(${target} output_name)
        if (WIN32)
            install(FILES $<TARGET_PDB_FILE:${target}> DESTINATION ${CMAKE_INSTALL_PREFIX} OPTIONAL)
        elseif (APPLE)
            get_target_prefix(${target} target_prefix)
            install(CODE "execute_process(WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND rm -fr \"$<TARGET_FILE_NAME:${target}>.dSYM\")")
            install(CODE "execute_process(WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND dsymutil \"${target_prefix}$<TARGET_FILE_NAME:${target}>\")")
            install(CODE "execute_process(WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND strip -S \"${target_prefix}$<TARGET_FILE_NAME:${target}>\")")
            if (NOT target_prefix STREQUAL "")
                install(CODE "execute_process(WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND mv \"${target_prefix}$<TARGET_FILE_NAME:${target}>.dSYM\" .)")
            endif ()
        elseif (UNIX)
            get_target_prefix(${target} target_prefix)
            install(CODE "execute_process(WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND objcopy --only-keep-debug \"${target_prefix}$<TARGET_FILE_NAME:${target}>\" \"${target_prefix}$<TARGET_FILE_NAME:${target}>.dbg\")")
            install(CODE "execute_process(WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND objcopy --strip-debug --add-gnu-debuglink=$<TARGET_FILE_NAME:${target}>.dbg \"${target_prefix}$<TARGET_FILE_NAME:${target}>\")")
        endif ()
        list(APPEND DEBUG_FILES_LIST "${output_name}")
    endforeach ()
    # This is a terrible hack to only run this on CI
    # until I turn this into a script or something
    if(DEFINED ENV{GITHUB_ENV})
        if (WIN32)
            list(TRANSFORM DEBUG_FILES_LIST APPEND ".pdb")
            list(JOIN DEBUG_FILES_LIST " " DEBUG_FILES)
            install(CODE "execute_process(WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND 7z a -t7z \"${CMAKE_INSTALL_PREFIX}/debug.7z\" ${DEBUG_FILES})")
        elseif (APPLE)
            list(TRANSFORM DEBUG_FILES_LIST APPEND ".dSYM")
            list(JOIN DEBUG_FILES_LIST " " DEBUG_FILES)
            install(CODE "execute_process(WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND tar acfv \"${CMAKE_INSTALL_PREFIX}/debug.tar.xz\" ${DEBUG_FILES})")
        elseif (UNIX)
            list(TRANSFORM DEBUG_FILES_LIST APPEND ".dbg")
            list(JOIN DEBUG_FILES_LIST " " DEBUG_FILES)
            install(CODE "execute_process(WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND tar -I \"xz -9 -T0\" -cvf \"${CMAKE_INSTALL_PREFIX}/debug.tar.xz\" ${DEBUG_FILES})")
        endif ()
    endif ()
endif ()
foreach (target IN LISTS BINARY_TARGETS)
    get_target_prefix(${target} target_prefix)
    foreach (extra_target IN LISTS EXTRA_TARGETS)
        get_target_prefix(${extra_target} extra_prefix)
        if (NOT "${target_prefix}" STREQUAL "${extra_prefix}")
            # Copy extra target to target prefix
            install(CODE "execute_process(WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND cp \"${extra_prefix}$<TARGET_FILE_NAME:${extra_target}>\" \"${target_prefix}$<TARGET_FILE_NAME:${extra_target}>\")")
        endif ()
    endforeach ()
endforeach ()
