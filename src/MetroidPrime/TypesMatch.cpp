#include "MetroidPrime/TCastTo.hpp"

#include "MetroidPrime/CEntity.hpp"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/CEffect.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Weapons/CBomb.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/ScriptObjects/CFishCloud.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/Enemies/CMetroidPrimeRelay.hpp"
#include "MetroidPrime/Cameras/CPathCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CRepulsor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActorKeyframe.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAiJumpPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraPitchVolume.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDebugCameraWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDistanceFog.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDock.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGrapplePoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGunTurret.hpp"
#include "MetroidPrime/ScriptObjects/CScriptMazeNode.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPickup.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPointOfInterest.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRoomAcoustics.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSound.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpawnPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpecialFunction.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpiderBallAttractionSurface.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpiderBallWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTargetingPoint.hpp"
#include "MetroidPrime/Enemies/CTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTimer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptVisorFlare.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CSnakeWeedSwarm.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpindleCamera.hpp"
#include "MetroidPrime/Enemies/CWallCrawlerSwarm.hpp"
#include "MetroidPrime/Enemies/CWallWalker.hpp"
#include "MetroidPrime/Enemies/CAtomicAlpha.hpp"
#include "MetroidPrime/Enemies/CAtomicBeta.hpp"
#include "MetroidPrime/Enemies/CBabygoth.hpp"
#include "MetroidPrime/Enemies/CBeetle.hpp"
#include "MetroidPrime/Enemies/CBloodFlower.hpp"
#include "MetroidPrime/Enemies/CBurrower.hpp"
#include "MetroidPrime/Enemies/CChozoGhost.hpp"
#include "MetroidPrime/Enemies/CDrone.hpp"
#include "MetroidPrime/Enemies/CElitePirate.hpp"
#include "MetroidPrime/Enemies/CEnergyBall.hpp"
#include "MetroidPrime/Enemies/CEyeBall.hpp"
#include "MetroidPrime/Enemies/CFireFlea.hpp"
#include "MetroidPrime/Enemies/CFlaahgra.hpp"
#include "MetroidPrime/Enemies/CFlaahgraTentacle.hpp"
#include "MetroidPrime/Enemies/CFlickerBat.hpp"
#include "MetroidPrime/Enemies/CFlyingPirate.hpp"
#include "MetroidPrime/Enemies/CIceSheegoth.hpp"
#include "MetroidPrime/Enemies/CJellyZap.hpp"
#include "MetroidPrime/Enemies/CMagdolite.hpp"
#include "MetroidPrime/Enemies/CMetaree.hpp"
#include "MetroidPrime/Enemies/CMetroid.hpp"
#include "MetroidPrime/Enemies/CMetroidBeta.hpp"
#include "MetroidPrime/Enemies/CMetroidPrime.hpp"
#include "MetroidPrime/Enemies/CNewIntroBoss.hpp"
#include "MetroidPrime/Enemies/CParasite.hpp"
#include "MetroidPrime/Enemies/CPuddleSpore.hpp"
#include "MetroidPrime/Enemies/CPuddleToadGamma.hpp"
#include "MetroidPrime/Enemies/CPuffer.hpp"
#include "MetroidPrime/Enemies/CRipper.hpp"
#include "MetroidPrime/Enemies/CRidley.hpp"
#include "MetroidPrime/Enemies/CSeedling.hpp"
#include "MetroidPrime/Enemies/CSpacePirate.hpp"
#include "MetroidPrime/Enemies/CSpankWeed.hpp"
#include "MetroidPrime/Enemies/CThardus.hpp"
#include "MetroidPrime/Enemies/CThardusRockProjectile.hpp"
#include "MetroidPrime/Enemies/CTryclops.hpp"
#include "MetroidPrime/Enemies/CWarWasp.hpp"

#define ID_CEntity 0
#define ID_CActor 1
#define ID_CGameCamera 2
#define ID_CPhysicsActor 3
#define ID_CWeapon 4
#define ID_CAi 5
#define ID_CEffect 6
#define ID_CPatterned 7
#define ID_CGameProjectile 8
#define ID_CBallCamera 9
#define ID_CBomb 10
#define ID_CCinematicCamera 11
#define ID_CCollisionActor 12
#define ID_CDestroyableRock 13
#define ID_CEnergyProjectile 14
#define ID_CExplosion 15
#define ID_CFirstPersonCamera 16
#define ID_CFishCloud 17
#define ID_CGameLight 18
#define ID_CHUDBillboardEffect 19
#define ID_CMetroidPrimeRelay 20
#define ID_CPathCamera 21
#define ID_CPlayer 22
#define ID_CRepulsor 23
#define ID_CScriptActor 24
#define ID_CScriptActorKeyframe 25
#define ID_CScriptAiJumpPoint 26
#define ID_CScriptCameraHint 27
#define ID_CScriptCameraPitchVolume 28
#define ID_CScriptCameraWaypoint 29
#define ID_CScriptCoverPoint 30
#define ID_CScriptDebugCameraWaypoint 31
#define ID_CScriptDistanceFog 32
#define ID_CScriptDock 33
#define ID_CScriptDoor 34
#define ID_CScriptEffect 35
#define ID_CScriptGrapplePoint 36
#define ID_CScriptGunTurret 37
#define ID_CScriptMazeNode 38
#define ID_CScriptPickup 39
#define ID_CScriptPlatform 40
#define ID_CScriptPlayerHint 41
#define ID_CScriptPointOfInterest 42
#define ID_CScriptRoomAcoustics 43
#define ID_CScriptSound 44
#define ID_CScriptSpawnPoint 45
#define ID_CScriptSpecialFunction 46
#define ID_CScriptSpiderBallAttractionSurface 47
#define ID_CScriptSpiderBallWaypoint 48
#define ID_CScriptTargetingPoint 49
#define ID_CTeamAiMgr 50
#define ID_CScriptTimer 51
#define ID_CScriptTrigger 52
#define ID_CScriptVisorFlare 53
#define ID_CScriptWater 54
#define ID_CScriptWaypoint 55
#define ID_CSnakeWeedSwarm 56
#define ID_CScriptSpindleCamera 57
#define ID_CWallCrawlerSwarm 58
#define ID_CWallWalker 59
#define ID_CAtomicAlpha 60
#define ID_CAtomicBeta 61
#define ID_CBabygoth 62
#define ID_CBeetle 63
#define ID_CBloodFlower 64
#define ID_CBurrower 65
#define ID_CChozoGhost 66
#define ID_CDrone 67
#define ID_CElitePirate 68
#define ID_CEnergyBall 69
#define ID_CEyeBall 70
#define ID_CFireFlea 71
#define ID_CFlaahgra 72
#define ID_CFlaahgraTentacle 73
#define ID_CFlickerBat 74
#define ID_CFlyingPirate 75
#define ID_CIceSheegoth 76
#define ID_CJellyZap 77
#define ID_CMagdolite 78
#define ID_CMetaree 79
#define ID_CMetroid 80
#define ID_CMetroidBeta 81
#define ID_CMetroidPrime 82
#define ID_CNewIntroBoss 83
#define ID_CParasite 84
#define ID_CPuddleSpore 85
#define ID_CPuddleToadGamma 86
#define ID_CPuffer 87
#define ID_CRipper 88
#define ID_CRidley 89
#define ID_CSeedling 90
#define ID_CSpacePirate 91
#define ID_CSpankWeed 92
#define ID_CThardus 93
#define ID_CThardusRockProjectile 94
#define ID_CTryclops 95
#define ID_CWarWasp 96

CEntity* TryCast(CEntity* entity, int type) {
  if (entity != nullptr) {
    return entity->TypesMatch(type);
  }
  return nullptr;
}

#define TYPES_MATCH_IMPL(CLS, PARENT) \
CEntity* CLS::TypesMatch(int type) { \
  if (type == ID_##CLS) return this; \
  if (type > ID_##CLS) return nullptr; \
  return PARENT::TypesMatch(type); \
}

CEntity* CEntity::TypesMatch(int type) {
  if (type == 0) {
    return this;
  }
  return nullptr;
}

TYPES_MATCH_IMPL(CActor, CEntity)
TYPES_MATCH_IMPL(CGameCamera, CActor)
TYPES_MATCH_IMPL(CPhysicsActor, CActor)
TYPES_MATCH_IMPL(CWeapon, CActor)
TYPES_MATCH_IMPL(CAi, CPhysicsActor)
TYPES_MATCH_IMPL(CEffect, CActor)
TYPES_MATCH_IMPL(CPatterned, CAi)
TYPES_MATCH_IMPL(CGameProjectile, CWeapon)
TYPES_MATCH_IMPL(CBallCamera, CGameCamera)
TYPES_MATCH_IMPL(CBomb, CWeapon)
TYPES_MATCH_IMPL(CCinematicCamera, CGameCamera)
TYPES_MATCH_IMPL(CCollisionActor, CPhysicsActor)
TYPES_MATCH_IMPL(CDestroyableRock, CAi)
TYPES_MATCH_IMPL(CEnergyProjectile, CGameProjectile)
TYPES_MATCH_IMPL(CExplosion, CEffect)
TYPES_MATCH_IMPL(CFirstPersonCamera, CGameCamera)
TYPES_MATCH_IMPL(CFishCloud, CActor)
TYPES_MATCH_IMPL(CGameLight, CActor)
TYPES_MATCH_IMPL(CHUDBillboardEffect, CEffect)
TYPES_MATCH_IMPL(CMetroidPrimeRelay, CEntity)
TYPES_MATCH_IMPL(CPathCamera, CGameCamera)
TYPES_MATCH_IMPL(CPlayer, CPhysicsActor)
TYPES_MATCH_IMPL(CRepulsor, CActor)
TYPES_MATCH_IMPL(CScriptActor, CPhysicsActor)
TYPES_MATCH_IMPL(CScriptActorKeyframe, CEntity)
TYPES_MATCH_IMPL(CScriptAiJumpPoint, CActor)
TYPES_MATCH_IMPL(CScriptCameraHint, CActor)
TYPES_MATCH_IMPL(CScriptCameraPitchVolume, CActor)
TYPES_MATCH_IMPL(CScriptCameraWaypoint, CActor)
TYPES_MATCH_IMPL(CScriptCoverPoint, CActor)
TYPES_MATCH_IMPL(CScriptDebugCameraWaypoint, CActor)
TYPES_MATCH_IMPL(CScriptDistanceFog, CEntity)
TYPES_MATCH_IMPL(CScriptDock, CPhysicsActor)
TYPES_MATCH_IMPL(CScriptDoor, CPhysicsActor)
TYPES_MATCH_IMPL(CScriptEffect, CActor)
TYPES_MATCH_IMPL(CScriptGrapplePoint, CActor)
TYPES_MATCH_IMPL(CScriptGunTurret, CPhysicsActor)
TYPES_MATCH_IMPL(CScriptMazeNode, CEntity)
TYPES_MATCH_IMPL(CScriptPickup, CPhysicsActor)
TYPES_MATCH_IMPL(CScriptPlatform, CPhysicsActor)
TYPES_MATCH_IMPL(CScriptPlayerHint, CActor)
TYPES_MATCH_IMPL(CScriptPointOfInterest, CActor)
TYPES_MATCH_IMPL(CScriptRoomAcoustics, CEntity)
TYPES_MATCH_IMPL(CScriptSound, CActor)
TYPES_MATCH_IMPL(CScriptSpawnPoint, CEntity)
TYPES_MATCH_IMPL(CScriptSpecialFunction, CActor)
TYPES_MATCH_IMPL(CScriptSpiderBallAttractionSurface, CActor)
TYPES_MATCH_IMPL(CScriptSpiderBallWaypoint, CActor)
TYPES_MATCH_IMPL(CScriptTargetingPoint, CActor)
TYPES_MATCH_IMPL(CTeamAiMgr, CEntity)
TYPES_MATCH_IMPL(CScriptTimer, CEntity)
TYPES_MATCH_IMPL(CScriptTrigger, CActor)
TYPES_MATCH_IMPL(CScriptVisorFlare, CActor)
TYPES_MATCH_IMPL(CScriptWater, CScriptTrigger)
TYPES_MATCH_IMPL(CScriptWaypoint, CActor)
TYPES_MATCH_IMPL(CSnakeWeedSwarm, CActor)
TYPES_MATCH_IMPL(CScriptSpindleCamera, CGameCamera)
TYPES_MATCH_IMPL(CWallCrawlerSwarm, CActor)
TYPES_MATCH_IMPL(CWallWalker, CPatterned)
TYPES_MATCH_IMPL(CAtomicAlpha, CPatterned)
TYPES_MATCH_IMPL(CAtomicBeta, CPatterned)
TYPES_MATCH_IMPL(CBabygoth, CPatterned)
TYPES_MATCH_IMPL(CBeetle, CPatterned)
TYPES_MATCH_IMPL(CBloodFlower, CPatterned)
TYPES_MATCH_IMPL(CBurrower, CPatterned)
TYPES_MATCH_IMPL(CChozoGhost, CPatterned)
TYPES_MATCH_IMPL(CDrone, CPatterned)
TYPES_MATCH_IMPL(CElitePirate, CPatterned)
TYPES_MATCH_IMPL(CEnergyBall, CPatterned)
TYPES_MATCH_IMPL(CEyeBall, CPatterned)
TYPES_MATCH_IMPL(CFireFlea, CPatterned)
TYPES_MATCH_IMPL(CFlaahgra, CPatterned)
TYPES_MATCH_IMPL(CFlaahgraTentacle, CPatterned)
TYPES_MATCH_IMPL(CFlickerBat, CPatterned)
TYPES_MATCH_IMPL(CFlyingPirate, CPatterned)
TYPES_MATCH_IMPL(CIceSheegoth, CPatterned)
TYPES_MATCH_IMPL(CJellyZap, CPatterned)
TYPES_MATCH_IMPL(CMagdolite, CPatterned)
TYPES_MATCH_IMPL(CMetaree, CPatterned)
TYPES_MATCH_IMPL(CMetroid, CPatterned)
TYPES_MATCH_IMPL(CMetroidBeta, CPatterned)
TYPES_MATCH_IMPL(CMetroidPrime, CPatterned)
TYPES_MATCH_IMPL(CNewIntroBoss, CPatterned)
TYPES_MATCH_IMPL(CParasite, CWallWalker)
TYPES_MATCH_IMPL(CPuddleSpore, CPatterned)
TYPES_MATCH_IMPL(CPuddleToadGamma, CPatterned)
TYPES_MATCH_IMPL(CPuffer, CPatterned)
TYPES_MATCH_IMPL(CRipper, CPatterned)
TYPES_MATCH_IMPL(CRidley, CPatterned)
TYPES_MATCH_IMPL(CSeedling, CWallWalker)
TYPES_MATCH_IMPL(CSpacePirate, CPatterned)
TYPES_MATCH_IMPL(CSpankWeed, CPatterned)
TYPES_MATCH_IMPL(CThardus, CPatterned)
TYPES_MATCH_IMPL(CThardusRockProjectile, CPatterned)
TYPES_MATCH_IMPL(CTryclops, CPatterned)
TYPES_MATCH_IMPL(CWarWasp, CPatterned)

#define CAST_TO_PTR_IMPL(CLS) \
template <> \
CLS* TCastToPtr< CLS >(CEntity* entity) { \
  return static_cast< CLS* >(TryCast(entity, ID_##CLS)); \
}

#define CAST_TO_REF_IMPL(CLS) \
template <> \
CLS* TCastToPtr< CLS >(CEntity& entity) { \
  return static_cast< CLS* >(entity.TypesMatch(ID_##CLS)); \
}

CAST_TO_PTR_IMPL(CEntity)
CAST_TO_REF_IMPL(CActor)
CAST_TO_PTR_IMPL(CActor)
CAST_TO_REF_IMPL(CGameCamera)
CAST_TO_PTR_IMPL(CGameCamera)
CAST_TO_REF_IMPL(CPhysicsActor)
CAST_TO_PTR_IMPL(CPhysicsActor)
CAST_TO_REF_IMPL(CWeapon)
CAST_TO_PTR_IMPL(CWeapon)
CAST_TO_REF_IMPL(CPatterned)
CAST_TO_PTR_IMPL(CPatterned)
CAST_TO_REF_IMPL(CGameProjectile)
CAST_TO_PTR_IMPL(CGameProjectile)
CAST_TO_PTR_IMPL(CBomb)
CAST_TO_REF_IMPL(CCinematicCamera)
CAST_TO_PTR_IMPL(CCinematicCamera)
CAST_TO_REF_IMPL(CCollisionActor)
CAST_TO_PTR_IMPL(CCollisionActor)
CAST_TO_REF_IMPL(CDestroyableRock)
CAST_TO_REF_IMPL(CEnergyProjectile)
CAST_TO_PTR_IMPL(CEnergyProjectile)
CAST_TO_PTR_IMPL(CExplosion)
CAST_TO_REF_IMPL(CFirstPersonCamera)
CAST_TO_PTR_IMPL(CFishCloud)
CAST_TO_REF_IMPL(CGameLight)
CAST_TO_PTR_IMPL(CGameLight)
CAST_TO_PTR_IMPL(CHUDBillboardEffect)
CAST_TO_PTR_IMPL(CMetroidPrimeRelay)
CAST_TO_PTR_IMPL(CPathCamera)
CAST_TO_REF_IMPL(CPlayer)
CAST_TO_PTR_IMPL(CPlayer)
CAST_TO_PTR_IMPL(CRepulsor)
CAST_TO_PTR_IMPL(CScriptActor)
CAST_TO_PTR_IMPL(CScriptActorKeyframe)
CAST_TO_REF_IMPL(CScriptAiJumpPoint)
CAST_TO_PTR_IMPL(CScriptAiJumpPoint)
CAST_TO_PTR_IMPL(CScriptCameraHint)
CAST_TO_PTR_IMPL(CScriptCameraPitchVolume)
CAST_TO_PTR_IMPL(CScriptCameraWaypoint)
CAST_TO_REF_IMPL(CScriptCoverPoint)
CAST_TO_PTR_IMPL(CScriptCoverPoint)
CAST_TO_PTR_IMPL(CScriptDistanceFog)
CAST_TO_PTR_IMPL(CScriptDock)
CAST_TO_PTR_IMPL(CScriptDoor)
CAST_TO_PTR_IMPL(CScriptEffect)
CAST_TO_PTR_IMPL(CScriptGrapplePoint)
CAST_TO_PTR_IMPL(CScriptGunTurret)
CAST_TO_PTR_IMPL(CScriptMazeNode)
CAST_TO_PTR_IMPL(CScriptPickup)
CAST_TO_REF_IMPL(CScriptPlatform)
CAST_TO_PTR_IMPL(CScriptPlatform)
CAST_TO_PTR_IMPL(CScriptPlayerHint)
CAST_TO_PTR_IMPL(CScriptRoomAcoustics)
CAST_TO_PTR_IMPL(CScriptSound)
CAST_TO_PTR_IMPL(CScriptSpawnPoint)
CAST_TO_PTR_IMPL(CScriptSpiderBallAttractionSurface)
CAST_TO_PTR_IMPL(CScriptSpiderBallWaypoint)
CAST_TO_PTR_IMPL(CScriptTargetingPoint)
CAST_TO_PTR_IMPL(CTeamAiMgr)
CAST_TO_PTR_IMPL(CScriptTimer)
CAST_TO_REF_IMPL(CScriptTrigger)
CAST_TO_PTR_IMPL(CScriptTrigger)
CAST_TO_PTR_IMPL(CScriptVisorFlare)
CAST_TO_REF_IMPL(CScriptWater)
CAST_TO_PTR_IMPL(CScriptWater)
CAST_TO_PTR_IMPL(CScriptWaypoint)
CAST_TO_PTR_IMPL(CSnakeWeedSwarm)
CAST_TO_PTR_IMPL(CScriptSpindleCamera)
CAST_TO_PTR_IMPL(CWallCrawlerSwarm)
CAST_TO_PTR_IMPL(CEnergyBall)
CAST_TO_PTR_IMPL(CFlickerBat)
CAST_TO_PTR_IMPL(CIceSheegoth)
CAST_TO_PTR_IMPL(CJellyZap)
CAST_TO_REF_IMPL(CMetroid)
CAST_TO_PTR_IMPL(CMetroid)
CAST_TO_PTR_IMPL(CMetroidBeta)
CAST_TO_PTR_IMPL(CMetroidPrime)
CAST_TO_PTR_IMPL(CParasite)
CAST_TO_REF_IMPL(CPuddleToadGamma)
CAST_TO_PTR_IMPL(CSpacePirate)
CAST_TO_PTR_IMPL(CThardusRockProjectile)
CAST_TO_PTR_IMPL(CWarWasp)
