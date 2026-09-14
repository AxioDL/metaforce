#ifndef _TCASTTO
#define _TCASTTO

#include "GameVersions.h"
#include "types.h"

class CEntity;

// clang-format off
#define TCASTTO_VISITORS \
VISIT(CActor) \
VISIT(CBallCamera) \
VISIT(CBomb) \
VISIT(CCinematicCamera) \
VISIT(CCollisionActor) \
VISIT(CDestroyableRock) \
VISIT(CEnergyProjectile) \
VISIT(CEntity) \
VISIT(CExplosion) \
VISIT(CFirstPersonCamera) \
VISIT(CFishCloud) \
VISIT(CGameCamera) \
VISIT(CGameLight) \
VISIT(CGameProjectile) \
VISIT(CHUDBillboardEffect) \
VISIT(CMetroidPrimeRelay) \
VISIT(CPathCamera) \
VISIT(CPatterned) \
VISIT(CPhysicsActor) \
VISIT(CPlayer) \
VISIT(CRepulsor) \
VISIT(CScriptActor) \
VISIT(CScriptActorKeyframe) \
VISIT(CScriptAiJumpPoint) \
VISIT(CScriptCameraHint) \
VISIT(CScriptCameraPitchVolume) \
VISIT(CScriptCameraWaypoint) \
VISIT(CScriptCoverPoint) \
VISIT(CScriptDebugCameraWaypoint) \
VISIT(CScriptDistanceFog) \
VISIT(CScriptDock) \
VISIT(CScriptDoor) \
VISIT(CScriptEffect) \
VISIT(CScriptGrapplePoint) \
VISIT(CScriptGunTurret) \
VISIT(CScriptMazeNode) \
VISIT(CScriptPickup) \
VISIT(CScriptPlatform) \
VISIT(CScriptPlayerHint) \
VISIT(CScriptPointOfInterest) \
VISIT(CScriptRoomAcoustics) \
VISIT(CScriptSound) \
VISIT(CScriptSpawnPoint) \
VISIT(CScriptSpecialFunction) \
VISIT(CScriptSpiderBallAttractionSurface) \
VISIT(CScriptSpiderBallWaypoint) \
VISIT(CScriptTargetingPoint) \
VISIT(CTeamAiMgr) \
VISIT(CScriptTimer) \
VISIT(CScriptTrigger) \
VISIT(CScriptVisorFlare) \
VISIT(CScriptWater) \
VISIT(CScriptWaypoint) \
VISIT(CSnakeWeedSwarm) \
VISIT(CScriptSpindleCamera) \
VISIT(CWallCrawlerSwarm) \
VISIT(CWeapon)
// clang-format on

#define VISIT(cls) class cls;
TCASTTO_VISITORS
#undef VISIT

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02

template < class T >
T* TCastToPtr(CEntity* p);

template < class T >
T* TCastToPtr(CEntity& p);

#define DECLARE_TYPES_MATCH virtual CEntity* TypesMatch(int type) override
#define DECLARE_TYPES_MATCH_OR_ACCEPT DECLARE_TYPES_MATCH
#define ENTITY_ACCEPT_IMPL(CLS)
#define HAS_TYPES_MATCH 1

#else
// NTSC 0-02 still uses the visitor API

class IVisitor {
public:
#define VISIT(cls) virtual void Visit(cls& p) = 0;
  TCASTTO_VISITORS
#undef VISIT
};

template < class T >
class TCastToPtr : public IVisitor {
public:
  TCastToPtr(CEntity* p);
  TCastToPtr(CEntity& p);
  operator T*() const { return ptr; }

#define VISIT(cls) void Visit(cls& p) override;
  TCASTTO_VISITORS
#undef VISIT

private:
  // Compiler picks the overload based on whether
  // the passed pointer is convertible to T*
  T* GetPtr(void* ptr) const { return nullptr; }
  T* GetPtr(T* ptr) const { return ptr; }

  T* ptr;
};


#define DECLARE_TYPES_MATCH
#define DECLARE_TYPES_MATCH_OR_ACCEPT void Accept(IVisitor& visitor) override
#define ENTITY_ACCEPT_IMPL(CLS) void CLS::Accept(IVisitor& visitor) { visitor.Visit(*this); }

#endif

template < typename T >
static inline const T* TCastToConstPtr(const CEntity* p) {
  return TCastToPtr< T >(const_cast< CEntity* >(p));
}
template < typename T >
static inline const T* TCastToConstPtr(const CEntity& p) {
  return TCastToPtr< T >(const_cast< CEntity& >(p));
}

#endif // _TCASTTO
