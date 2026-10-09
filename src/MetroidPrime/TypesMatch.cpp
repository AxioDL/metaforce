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
#include "MetroidPrime/ScriptObjects/CScriptCounter.hpp"
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

#include "MetroidPrime/CCollisionActorManager.hpp"
#include "WorldFormat/COBBTree.hpp"
#include "MetroidPrime/CProjectedShadow.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"

enum ETypeId {
  kTI_CEntity,
  kTI_CActor,
  kTI_CGameCamera,
  kTI_CPhysicsActor,
  kTI_CWeapon,
  kTI_CAi,
  kTI_CEffect,
  kTI_CPatterned,
  kTI_CGameProjectile,
  kTI_CBallCamera,
  kTI_CBomb,
  kTI_CCinematicCamera,
  kTI_CCollisionActor,
  kTI_CDestroyableRock,
  kTI_CEnergyProjectile,
  kTI_CExplosion,
  kTI_CFirstPersonCamera,
  kTI_CFishCloud,
  kTI_CGameLight,
  kTI_CHUDBillboardEffect,
  kTI_CMetroidPrimeRelay,
  kTI_CPathCamera,
  kTI_CPlayer,
  kTI_CRepulsor,
  kTI_CScriptActor,
  kTI_CScriptActorKeyframe,
  kTI_CScriptAiJumpPoint,
  kTI_CScriptCameraHint,
  kTI_CScriptCameraPitchVolume,
  kTI_CScriptCameraWaypoint,
#if VERSION >= VERSION_R3IJ_00
  kTI_CScriptCounter,
#endif
  kTI_CScriptCoverPoint,
  kTI_CScriptDebugCameraWaypoint,
  kTI_CScriptDistanceFog,
  kTI_CScriptDock,
  kTI_CScriptDoor,
  kTI_CScriptEffect,
  kTI_CScriptGrapplePoint,
  kTI_CScriptGunTurret,
  kTI_CScriptMazeNode,
  kTI_CScriptPickup,
  kTI_CScriptPlatform,
  kTI_CScriptPlayerHint,
  kTI_CScriptPointOfInterest,
  kTI_CScriptRoomAcoustics,
  kTI_CScriptSound,
  kTI_CScriptSpawnPoint,
  kTI_CScriptSpecialFunction,
  kTI_CScriptSpiderBallAttractionSurface,
  kTI_CScriptSpiderBallWaypoint,
#if VERSION >= VERSION_R3IJ_00
  kTI_CScriptStreamedMusic,
#endif
  kTI_CScriptTargetingPoint,
  kTI_CTeamAiMgr,
  kTI_CScriptTimer,
  kTI_CScriptTrigger,
  kTI_CScriptVisorFlare,
  kTI_CScriptWater,
  kTI_CScriptWaypoint,
  kTI_CSnakeWeedSwarm,
  kTI_CScriptSpindleCamera,
  kTI_CWallCrawlerSwarm,
  kTI_CWallWalker,
  kTI_CAtomicAlpha,
  kTI_CAtomicBeta,
  kTI_CBabygoth,
  kTI_CBeetle,
  kTI_CBloodFlower,
  kTI_CBurrower,
  kTI_CChozoGhost,
  kTI_CDrone,
  kTI_CElitePirate,
  kTI_CEnergyBall,
  kTI_CEyeBall,
  kTI_CFireFlea,
  kTI_CFlaahgra,
  kTI_CFlaahgraTentacle,
  kTI_CFlickerBat,
  kTI_CFlyingPirate,
  kTI_CIceSheegoth,
  kTI_CJellyZap,
  kTI_CMagdolite,
  kTI_CMetaree,
  kTI_CMetroid,
  kTI_CMetroidBeta,
  kTI_CMetroidPrime,
  kTI_CNewIntroBoss,
  kTI_CParasite,
  kTI_CPuddleSpore,
  kTI_CPuddleToadGamma,
  kTI_CPuffer,
  kTI_CRipper,
  kTI_CRidley,
  kTI_CSeedling,
  kTI_CSpacePirate,
  kTI_CSpankWeed,
  kTI_CThardus,
  kTI_CThardusRockProjectile,
  kTI_CTryclops,
  kTI_CWarWasp,
};

CEntity* TryCast(CEntity* entity, int type) {
  if (entity != nullptr) {
    return entity->TypesMatch(type);
  }
  return nullptr;
}

#define TYPES_MATCH_IMPL(CLS, PARENT) \
CEntity* CLS::TypesMatch(int type) { \
  if (type == kTI_##CLS) return this; \
  if (type > kTI_##CLS) return nullptr; \
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
#if VERSION >= VERSION_R3IJ_00
TYPES_MATCH_IMPL(CScriptCounter, CEntity)
#endif
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
  return static_cast< CLS* >(TryCast(entity, kTI_##CLS)); \
}

#define CAST_TO_REF_IMPL(CLS) \
template <> \
CLS* TCastToPtr< CLS >(CEntity& entity) { \
  return static_cast< CLS* >(entity.TypesMatch(kTI_##CLS)); \
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
#if VERSION >= VERSION_R3IJ_00
CAST_TO_REF_IMPL(CScriptCounter)
CAST_TO_PTR_IMPL(CScriptCounter)
#endif
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
#if VERSION == VERSION_GM8J_00
CAST_TO_PTR_IMPL(CScriptSpecialFunction)
#endif
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
