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



#define TYPES_MATCH_IMPL(CLS, PARENT, ID) \
CEntity* CLS::TypesMatch(int type) { \
  if (type == ID) return this; \
  if (type > ID) return nullptr; \
  return PARENT::TypesMatch(type); \
}

CEntity* CEntity::TypesMatch(int type) {
  if (type == 0) {
    return this;
  }
  return nullptr;
}

TYPES_MATCH_IMPL(CActor, CEntity, 1)
TYPES_MATCH_IMPL(CGameCamera, CActor, 2)
TYPES_MATCH_IMPL(CPhysicsActor, CActor, 3)
TYPES_MATCH_IMPL(CWeapon, CActor, 4)
TYPES_MATCH_IMPL(CAi, CPhysicsActor, 5)
TYPES_MATCH_IMPL(CEffect, CActor, 6)
TYPES_MATCH_IMPL(CPatterned, CAi, 7)
TYPES_MATCH_IMPL(CGameProjectile, CWeapon, 8)
TYPES_MATCH_IMPL(CBallCamera, CGameCamera, 9)
TYPES_MATCH_IMPL(CBomb, CWeapon, 10)
TYPES_MATCH_IMPL(CCinematicCamera, CGameCamera, 11)
TYPES_MATCH_IMPL(CCollisionActor, CPhysicsActor, 12)
TYPES_MATCH_IMPL(CDestroyableRock, CAi, 13)
TYPES_MATCH_IMPL(CEnergyProjectile, CGameProjectile, 14)
TYPES_MATCH_IMPL(CExplosion, CEffect, 15)
TYPES_MATCH_IMPL(CFirstPersonCamera, CGameCamera, 16)
TYPES_MATCH_IMPL(CFishCloud, CActor, 17)
TYPES_MATCH_IMPL(CGameLight, CActor, 18)
TYPES_MATCH_IMPL(CHUDBillboardEffect, CEffect, 19)
TYPES_MATCH_IMPL(CMetroidPrimeRelay, CEntity, 20)
TYPES_MATCH_IMPL(CPathCamera, CGameCamera, 21)
TYPES_MATCH_IMPL(CPlayer, CPhysicsActor, 22)
TYPES_MATCH_IMPL(CRepulsor, CActor, 23)
TYPES_MATCH_IMPL(CScriptActor, CPhysicsActor, 24)
TYPES_MATCH_IMPL(CScriptActorKeyframe, CEntity, 25)
TYPES_MATCH_IMPL(CScriptAiJumpPoint, CActor, 26)
TYPES_MATCH_IMPL(CScriptCameraHint, CActor, 27)
TYPES_MATCH_IMPL(CScriptCameraPitchVolume, CActor, 28)
TYPES_MATCH_IMPL(CScriptCameraWaypoint, CActor, 29)
TYPES_MATCH_IMPL(CScriptCoverPoint, CActor, 30)
TYPES_MATCH_IMPL(CScriptDebugCameraWaypoint, CActor, 31)
TYPES_MATCH_IMPL(CScriptDistanceFog, CEntity, 32)
TYPES_MATCH_IMPL(CScriptDock, CPhysicsActor, 33)
TYPES_MATCH_IMPL(CScriptDoor, CPhysicsActor, 34)
TYPES_MATCH_IMPL(CScriptEffect, CActor, 35)
TYPES_MATCH_IMPL(CScriptGrapplePoint, CActor, 36)
TYPES_MATCH_IMPL(CScriptGunTurret, CPhysicsActor, 37)
TYPES_MATCH_IMPL(CScriptMazeNode, CEntity, 38)
TYPES_MATCH_IMPL(CScriptPickup, CPhysicsActor, 39)
TYPES_MATCH_IMPL(CScriptPlatform, CPhysicsActor, 40)
TYPES_MATCH_IMPL(CScriptPlayerHint, CActor, 41)
TYPES_MATCH_IMPL(CScriptPointOfInterest, CActor, 42)
TYPES_MATCH_IMPL(CScriptRoomAcoustics, CEntity, 43)
TYPES_MATCH_IMPL(CScriptSound, CActor, 44)
TYPES_MATCH_IMPL(CScriptSpawnPoint, CEntity, 45)
TYPES_MATCH_IMPL(CScriptSpecialFunction, CActor, 46)
TYPES_MATCH_IMPL(CScriptSpiderBallAttractionSurface, CActor, 47)
TYPES_MATCH_IMPL(CScriptSpiderBallWaypoint, CActor, 48)
TYPES_MATCH_IMPL(CScriptTargetingPoint, CActor, 49)
TYPES_MATCH_IMPL(CTeamAiMgr, CEntity, 50)
TYPES_MATCH_IMPL(CScriptTimer, CEntity, 51)
TYPES_MATCH_IMPL(CScriptTrigger, CActor, 52)
TYPES_MATCH_IMPL(CScriptVisorFlare, CActor, 53)
TYPES_MATCH_IMPL(CScriptWater, CScriptTrigger, 54)
TYPES_MATCH_IMPL(CScriptWaypoint, CActor, 55)
TYPES_MATCH_IMPL(CSnakeWeedSwarm, CActor, 56)
TYPES_MATCH_IMPL(CScriptSpindleCamera, CGameCamera, 57)
TYPES_MATCH_IMPL(CWallCrawlerSwarm, CActor, 58)
TYPES_MATCH_IMPL(CWallWalker, CPatterned, 59)
TYPES_MATCH_IMPL(CAtomicAlpha, CPatterned, 60)
TYPES_MATCH_IMPL(CAtomicBeta, CPatterned, 61)
TYPES_MATCH_IMPL(CBabygoth, CPatterned, 62)
TYPES_MATCH_IMPL(CBeetle, CPatterned, 63)
TYPES_MATCH_IMPL(CBloodFlower, CPatterned, 64)
TYPES_MATCH_IMPL(CBurrower, CPatterned, 65)
TYPES_MATCH_IMPL(CChozoGhost, CPatterned, 66)
TYPES_MATCH_IMPL(CDrone, CPatterned, 67)
TYPES_MATCH_IMPL(CElitePirate, CPatterned, 68)
TYPES_MATCH_IMPL(CEnergyBall, CPatterned, 69)
TYPES_MATCH_IMPL(CEyeBall, CPatterned, 70)
TYPES_MATCH_IMPL(CFireFlea, CPatterned, 71)
TYPES_MATCH_IMPL(CFlaahgra, CPatterned, 72)
TYPES_MATCH_IMPL(CFlaahgraTentacle, CPatterned, 73)
TYPES_MATCH_IMPL(CFlickerBat, CPatterned, 74)
TYPES_MATCH_IMPL(CFlyingPirate, CPatterned, 75)
TYPES_MATCH_IMPL(CIceSheegoth, CPatterned, 76)
TYPES_MATCH_IMPL(CJellyZap, CPatterned, 77)
TYPES_MATCH_IMPL(CMagdolite, CPatterned, 78)
TYPES_MATCH_IMPL(CMetaree, CPatterned, 79)
TYPES_MATCH_IMPL(CMetroid, CPatterned, 80)
TYPES_MATCH_IMPL(CMetroidBeta, CPatterned, 81)
TYPES_MATCH_IMPL(CMetroidPrime, CPatterned, 82)
TYPES_MATCH_IMPL(CNewIntroBoss, CPatterned, 83)
TYPES_MATCH_IMPL(CParasite, CWallWalker, 84)
TYPES_MATCH_IMPL(CPuddleSpore, CPatterned, 85)
TYPES_MATCH_IMPL(CPuddleToadGamma, CPatterned, 86)
TYPES_MATCH_IMPL(CPuffer, CPatterned, 87)
TYPES_MATCH_IMPL(CRipper, CPatterned, 88)
TYPES_MATCH_IMPL(CRidley, CPatterned, 89)
TYPES_MATCH_IMPL(CSeedling, CWallWalker, 90)
TYPES_MATCH_IMPL(CSpacePirate, CPatterned, 91)
TYPES_MATCH_IMPL(CSpankWeed, CPatterned, 92)
TYPES_MATCH_IMPL(CThardus, CPatterned, 93)
TYPES_MATCH_IMPL(CThardusRockProjectile, CPatterned, 94)
TYPES_MATCH_IMPL(CTryclops, CPatterned, 95)
TYPES_MATCH_IMPL(CWarWasp, CPatterned, 96)
