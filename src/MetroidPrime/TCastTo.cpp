#include "MetroidPrime/TCastTo.hpp"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"

// TODO create headers for all entities
#define STUB(cls, parent)                                                                          \
  class cls : public parent {                                                                      \
  public:                                                                                          \
    ~cls() override;                                                                               \
    DECLARE_TYPES_MATCH_OR_ACCEPT;                                                       \
  };
STUB(CWeapon, CActor)
STUB(CBomb, CWeapon)
STUB(CCinematicCamera, CGameCamera)
STUB(CPhysicsActor, CActor)
STUB(CCollisionActor, CPhysicsActor)
STUB(CAi, CPhysicsActor)
STUB(CDestroyableRock, CAi)
STUB(CGameProjectile, CWeapon)
STUB(CEnergyProjectile, CGameProjectile)
STUB(CEffect, CActor)
STUB(CExplosion, CEffect)
STUB(CFirstPersonCamera, CGameCamera)
STUB(CFishCloud, CActor)
STUB(CGameLight, CActor)
STUB(CHUDBillboardEffect, CEffect)
STUB(CMetroidPrimeRelay, CEntity)
STUB(CPathCamera, CGameCamera)
STUB(CPatterned, CAi)
STUB(CPlayer, CPhysicsActor)
STUB(CRepulsor, CActor)
STUB(CScriptActor, CPhysicsActor)
STUB(CScriptActorKeyframe, CEntity)
STUB(CScriptAiJumpPoint, CActor)
STUB(CScriptCameraHint, CActor)
STUB(CScriptCameraPitchVolume, CActor)
STUB(CScriptCameraWaypoint, CActor)
STUB(CScriptCoverPoint, CActor)
STUB(CScriptDebugCameraWaypoint, CActor)
STUB(CScriptDistanceFog, CEntity)
STUB(CScriptDock, CPhysicsActor)
STUB(CScriptDoor, CPhysicsActor)
STUB(CScriptEffect, CActor)
STUB(CScriptGrapplePoint, CActor)
STUB(CScriptGunTurret, CPhysicsActor)
STUB(CScriptMazeNode, CActor)
STUB(CScriptPickup, CPhysicsActor)
STUB(CScriptPlatform, CPhysicsActor)
STUB(CScriptPlayerHint, CActor)
STUB(CScriptPointOfInterest, CActor)
STUB(CScriptRoomAcoustics, CEntity)
STUB(CScriptSound, CActor)
STUB(CScriptSpawnPoint, CEntity)
STUB(CScriptSpecialFunction, CActor)
STUB(CScriptSpiderBallAttractionSurface, CActor)
STUB(CScriptSpiderBallWaypoint, CActor)
STUB(CScriptTargetingPoint, CActor)
STUB(CTeamAiMgr, CEntity)
STUB(CScriptTimer, CEntity)
STUB(CScriptTrigger, CActor)
STUB(CScriptVisorFlare, CActor)
STUB(CScriptWater, CScriptTrigger)
STUB(CScriptWaypoint, CActor)
STUB(CSnakeWeedSwarm, CActor)
STUB(CScriptSpindleCamera, CGameCamera)
STUB(CWallCrawlerSwarm, CActor)

// MWCC gives up on expanding this macro, so the preprocessed version is included below
#ifndef __MWERKS__

// clang-format off
#define TCASTTO_VISITORS_IMPL(cls) \
VISIT_IMPL(cls, CActor) \
VISIT_IMPL(cls, CBallCamera) /* unused */ \
VISIT_IMPL(cls, CBomb) \
VISIT_IMPL(cls, CCinematicCamera) \
VISIT_IMPL(cls, CCollisionActor) \
VISIT_IMPL(cls, CDestroyableRock) \
VISIT_IMPL(cls, CEnergyProjectile) \
VISIT_IMPL(cls, CEntity) \
VISIT_IMPL(cls, CExplosion) \
VISIT_IMPL(cls, CFirstPersonCamera) \
VISIT_IMPL(cls, CFishCloud) \
VISIT_IMPL(cls, CGameCamera) \
VISIT_IMPL(cls, CGameLight) \
VISIT_IMPL(cls, CGameProjectile) \
VISIT_IMPL(cls, CHUDBillboardEffect) \
VISIT_IMPL(cls, CMetroidPrimeRelay) \
VISIT_IMPL(cls, CPathCamera) \
VISIT_IMPL(cls, CPatterned) \
VISIT_IMPL(cls, CPhysicsActor) \
VISIT_IMPL(cls, CPlayer) \
VISIT_IMPL(cls, CRepulsor) \
VISIT_IMPL(cls, CScriptActor) \
VISIT_IMPL(cls, CScriptActorKeyframe) \
VISIT_IMPL(cls, CScriptAiJumpPoint) \
VISIT_IMPL(cls, CScriptCameraHint) \
VISIT_IMPL(cls, CScriptCameraPitchVolume) \
VISIT_IMPL(cls, CScriptCameraWaypoint) \
VISIT_IMPL(cls, CScriptCoverPoint) \
VISIT_IMPL(cls, CScriptDebugCameraWaypoint) \
VISIT_IMPL(cls, CScriptDistanceFog) \
VISIT_IMPL(cls, CScriptDock) \
VISIT_IMPL(cls, CScriptDoor) \
VISIT_IMPL(cls, CScriptEffect) \
VISIT_IMPL(cls, CScriptGrapplePoint) \
VISIT_IMPL(cls, CScriptGunTurret) \
VISIT_IMPL(cls, CScriptMazeNode) \
VISIT_IMPL(cls, CScriptPickup) \
VISIT_IMPL(cls, CScriptPlatform) \
VISIT_IMPL(cls, CScriptPlayerHint) \
VISIT_IMPL(cls, CScriptPointOfInterest) /* unused */ \
VISIT_IMPL(cls, CScriptRoomAcoustics) \
VISIT_IMPL(cls, CScriptSound) \
VISIT_IMPL(cls, CScriptSpawnPoint) \
VISIT_IMPL(cls, CScriptSpecialFunction) /* unused */ \
VISIT_IMPL(cls, CScriptSpiderBallAttractionSurface) \
VISIT_IMPL(cls, CScriptSpiderBallWaypoint) \
VISIT_IMPL(cls, CScriptTargetingPoint) \
VISIT_IMPL(cls, CTeamAiMgr) \
VISIT_IMPL(cls, CScriptTimer) \
VISIT_IMPL(cls, CScriptTrigger) \
VISIT_IMPL(cls, CScriptVisorFlare) \
VISIT_IMPL(cls, CScriptWater) \
VISIT_IMPL(cls, CScriptWaypoint) \
VISIT_IMPL(cls, CSnakeWeedSwarm) \
VISIT_IMPL(cls, CScriptSpindleCamera) \
VISIT_IMPL(cls, CWallCrawlerSwarm) \
VISIT_IMPL(cls, CWeapon)
// clang-format on

#define VISIT_IMPL(base, cls)                                                                      \
  template <>                                                                                      \
  void TCastToPtr< base >::Visit(cls& v) {                                                         \
    ptr = GetPtr(&v);                                                                              \
  }
#define VISIT(cls)                                                                                 \
  template <>                                                                                      \
  TCastToPtr< cls >::TCastToPtr(CEntity& p) : ptr(nullptr) {                                       \
    p.Accept(*this);                                                                               \
  }                                                                                                \
  template <>                                                                                      \
  TCastToPtr< cls >::TCastToPtr(CEntity* p) : ptr(nullptr) {                                       \
    if (p != nullptr) {                                                                            \
      p->Accept(*this);                                                                            \
    }                                                                                              \
  }                                                                                                \
  TCASTTO_VISITORS_IMPL(cls)
TCASTTO_VISITORS
#undef VISIT
#undef VISIT_IMPL

#else
// Generated with gcc -I include -E src/MetroidPrime/TCastTo.cpp > TCastTo.i

template <>
TCastToPtr< CActor >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CActor >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CActor >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CActor >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CBallCamera >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CBallCamera >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CBallCamera >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBallCamera >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CBomb >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CBomb >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CBomb >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CBomb >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CCinematicCamera >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CCinematicCamera >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCinematicCamera >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CCollisionActor >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CCollisionActor >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CCollisionActor >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CDestroyableRock >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CDestroyableRock >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CDestroyableRock >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CEnergyProjectile >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CEnergyProjectile >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEnergyProjectile >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CEntity >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CEntity >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CEntity >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CEntity >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CExplosion >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CExplosion >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CExplosion >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CExplosion >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CFirstPersonCamera >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CFirstPersonCamera >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFirstPersonCamera >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CFishCloud >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CFishCloud >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CFishCloud >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CFishCloud >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CGameCamera >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CGameCamera >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CGameCamera >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameCamera >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CGameLight >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CGameLight >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CGameLight >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameLight >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CGameProjectile >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CGameProjectile >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CGameProjectile >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CHUDBillboardEffect >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CHUDBillboardEffect >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CHUDBillboardEffect >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CMetroidPrimeRelay >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CMetroidPrimeRelay >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CMetroidPrimeRelay >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CPathCamera >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CPathCamera >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CPathCamera >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPathCamera >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CPatterned >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CPatterned >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CPatterned >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPatterned >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CPhysicsActor >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CPhysicsActor >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPhysicsActor >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CPlayer >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CPlayer >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CPlayer >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CPlayer >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CRepulsor >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CRepulsor >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CRepulsor >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CRepulsor >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptActor >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptActor >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptActor >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActor >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptActorKeyframe >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptActorKeyframe >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptActorKeyframe >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptAiJumpPoint >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptAiJumpPoint >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptAiJumpPoint >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptCameraHint >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptCameraHint >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraHint >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptCameraPitchVolume >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptCameraPitchVolume >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraPitchVolume >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptCameraWaypoint >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptCameraWaypoint >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCameraWaypoint >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptCoverPoint >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptCoverPoint >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptCoverPoint >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptDebugCameraWaypoint >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptDebugCameraWaypoint >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDebugCameraWaypoint >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptDistanceFog >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptDistanceFog >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDistanceFog >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptDock >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptDock >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptDock >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDock >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptDoor >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptDoor >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptDoor >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptEffect >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptEffect >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptEffect >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptGrapplePoint >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptGrapplePoint >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGrapplePoint >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptGunTurret >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptGunTurret >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptGunTurret >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptMazeNode >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptMazeNode >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptMazeNode >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptPickup >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptPickup >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPickup >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptPlatform >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptPlatform >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlatform >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptPlayerHint >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptPlayerHint >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPlayerHint >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptPointOfInterest >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptPointOfInterest >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptPointOfInterest >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptRoomAcoustics >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptRoomAcoustics >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptRoomAcoustics >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptSound >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptSound >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptSound >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSound >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptSpawnPoint >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptSpawnPoint >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpawnPoint >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptSpecialFunction >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptSpecialFunction >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpecialFunction >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptSpiderBallAttractionSurface >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptSpiderBallAttractionSurface >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(
    CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallAttractionSurface >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptSpiderBallWaypoint >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptSpiderBallWaypoint >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpiderBallWaypoint >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptTargetingPoint >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptTargetingPoint >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTargetingPoint >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CTeamAiMgr >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CTeamAiMgr >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CTeamAiMgr >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptTimer >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptTimer >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTimer >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptTrigger >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptTrigger >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptTrigger >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptVisorFlare >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptVisorFlare >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptVisorFlare >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptWater >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptWater >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptWater >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWater >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptWaypoint >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptWaypoint >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptWaypoint >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CSnakeWeedSwarm >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CSnakeWeedSwarm >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CSnakeWeedSwarm >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CScriptSpindleCamera >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CScriptSpindleCamera >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CScriptSpindleCamera >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CWallCrawlerSwarm >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CWallCrawlerSwarm >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWallCrawlerSwarm >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}
template <>
TCastToPtr< CWeapon >::TCastToPtr(CEntity& p) : ptr(nullptr) {
  p.Accept(*this);
}
template <>
TCastToPtr< CWeapon >::TCastToPtr(CEntity* p) : ptr(nullptr) {
  if (p != nullptr) {
    p->Accept(*this);
  }
}
template <>
void TCastToPtr< CWeapon >::Visit(CActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CBallCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CBomb& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CCinematicCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CCollisionActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CDestroyableRock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CEnergyProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CEntity& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CExplosion& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CFirstPersonCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CFishCloud& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CGameCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CGameLight& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CGameProjectile& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CHUDBillboardEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CMetroidPrimeRelay& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CPathCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CPatterned& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CPhysicsActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CPlayer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CRepulsor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptActor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptActorKeyframe& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptAiJumpPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptCameraHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptCameraPitchVolume& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptCoverPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptDebugCameraWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptDistanceFog& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptDock& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptDoor& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptEffect& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptGrapplePoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptGunTurret& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptMazeNode& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptPickup& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptPlatform& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptPlayerHint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptPointOfInterest& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptRoomAcoustics& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptSound& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptSpawnPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptSpecialFunction& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptSpiderBallAttractionSurface& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptSpiderBallWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptTargetingPoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CTeamAiMgr& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptTimer& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptTrigger& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptVisorFlare& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptWater& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptWaypoint& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CSnakeWeedSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CScriptSpindleCamera& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CWallCrawlerSwarm& v) {
  ptr = GetPtr(&v);
}
template <>
void TCastToPtr< CWeapon >::Visit(CWeapon& v) {
  ptr = GetPtr(&v);
}

#endif
