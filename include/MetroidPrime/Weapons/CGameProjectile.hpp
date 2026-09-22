#ifndef _CGAMEPROJECTILE
#define _CGAMEPROJECTILE

#include "Collision/CRayCastResult.hpp"
#include "types.h"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Weapons/CProjectileWeapon.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"

#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/vector.hpp"

class CGenDescription;
class CLight;
class CProjectileTouchResult {
public:
  CProjectileTouchResult(const TUniqueId& id, const rstl::optional_object< CRayCastResult >& result)
  : x0_id(id), x4_result(result) {}
  TUniqueId GetActorId() const { return x0_id; }
  bool HasRayCastResult() const { return x4_result.valid(); }
  const CRayCastResult& GetRayCastResult() const { return *x4_result; }

private:
  TUniqueId x0_id;
  rstl::optional_object< CRayCastResult > x4_result;
};

CHECK_SIZEOF(CProjectileTouchResult, 0x38)

class CGameProjectile : public CWeapon {
public:
  CGameProjectile(const bool active, const TToken< CWeaponDescription >&, const rstl::string& name,
                  const EWeaponType wType, const CTransform4f& xf, const EMaterialTypes excludeMat,
                  const CDamageInfo& dInfo, const TUniqueId uid, const TAreaId aid,
                  const TUniqueId owner, const TUniqueId homingTarget, const uint attribs,
                  const bool underwater, const CVector3f& scale,
                  const rstl::optional_object< TLockedToken< CGenDescription > >& visorParticle,
                  const ushort visorSfx, bool sendCollideMsg);

  // CEntity
  ~CGameProjectile() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void Render(const CStateManager&) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void FluidFXThink(EFluidState, CScriptWater&, CStateManager&) override;

  // CGameProjectile
  virtual void ResolveCollisionWithActor(const CRayCastResult& res, CActor& act,
                                         CStateManager& mgr);

  CProjectileTouchResult CanCollideWithTrigger(CActor& act, CStateManager& mgr);
  const CProjectileWeapon& GetProjectile() const { return x170_projectile; }
  CProjectileWeapon& Projectile() { return x170_projectile; }
  CVector3f GetVelocity() const { return GetTranslation() - x298_previousPos; }
  const CVector3f& GetPreviousPos() const { return x298_previousPos; }
  TUniqueId GetProjectileLightId() const { return x2c8_projectileLight; }
  TUniqueId GetHomingTargetId() const { return x2c0_homingTargetId; }
  void SetHomingTargetId(TUniqueId id) { x2c0_homingTargetId = id; }
  TUniqueId GetHitProjectileOwner() const { return x2c4_hitProjectileOwner; }
  void SetHitProjectileOwner(TUniqueId id) { x2c4_hitProjectileOwner = id; }

  void SetLastObjectId(TUniqueId id) { x2c2_lastResolvedObj = id; }

  void SetMinHomingDistance(float distance) { x2e0_minHomingDist = distance; }

  bool GetWeaponActive() const { return x2e4_24_active; }
  void DeleteProjectileLight(CStateManager&);

#if VERSION >= VERSION_GM8P_00
  bool GetUnkPalFlag() const { return x2e4_29_unkPalFlag; }
#endif

  void SetUnkPalFlag(bool active) {
#if VERSION >= VERSION_GM8P_00
    x2e4_29_unkPalFlag = active; 
#endif
  }

  void ApplyDamageToActors(CStateManager& mgr, const CDamageInfo& dInfo);
  CRayCastResult RayCollisionCheckWithWorld(TUniqueId& idOut, const CVector3f& start,
                                            const CVector3f& end, float mag, TEntityList& nearList,
                                            CStateManager& mgr);
  static EProjectileAttrib GetBeamAttribType(EWeaponType wType);

  CAABox GetProjectileBounds() const;
  void CreateProjectileLight(const rstl::string& name, const CLight& light, CStateManager& mgr);
  void Chase(float dt, CStateManager& mgr);
  void UpdateHoming(float dt, CStateManager& mgr);
  void UpdateProjectileMovement(float dt, CStateManager& mgr);
  CRayCastResult DoCollisionCheck(TUniqueId& idOut, CStateManager& mgr);
  CProjectileTouchResult CanCollideWith(CActor& act, CStateManager& mgr);
  CProjectileTouchResult CanCollideWithComplexCollision(CActor& act, CStateManager& mgr);
  CProjectileTouchResult CanCollideWithGameObject(CActor& act, CStateManager& mgr);

  static const float kProjectileBoxAllowance;

protected:
  rstl::optional_object< TLockedToken< CGenDescription > > x158_visorParticle;
  ushort x168_visorSfx;
  CProjectileWeapon x170_projectile;
  CVector3f x298_previousPos;
  float x2a4_projExtent;
  float x2a8_homingDt;
  double x2b0_targetHomingTime;
  double x2b8_curHomingTime;
  TUniqueId x2c0_homingTargetId;
  TUniqueId x2c2_lastResolvedObj;
  TUniqueId x2c4_hitProjectileOwner;
  TUniqueId x2c6_pendingDamagee;
  TUniqueId x2c8_projectileLight;
  CAssetId x2cc_wpscId;
  rstl::vector< CProjectileTouchResult > x2d0_touchResults;
  float x2e0_minHomingDist;
  bool x2e4_24_active : 1;
  bool x2e4_25_startedUnderwater : 1;
  bool x2e4_26_waterUpdate : 1;
  bool x2e4_27_inWater : 1;
  bool x2e4_28_sendProjectileCollideMsg : 1;
#if VERSION >= VERSION_GM8P_00
  bool x2e4_29_unkPalFlag : 1;
#endif
};
CHECK_SIZEOF(CGameProjectile, (VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02 ? 0x310
                               : VERSION >= VERSION_GM8P_00                             ? 0x2f8
                                                                                        : 0x2e8))

#endif // _CGAMEPROJECTILE
