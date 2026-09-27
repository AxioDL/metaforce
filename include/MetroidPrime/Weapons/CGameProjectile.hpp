#ifndef _CGAMEPROJECTILE
#define _CGAMEPROJECTILE

#include "Collision/CRayCastResult.hpp"
#include "types.h"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"

#include "Kyoto/TToken.hpp"

#include "Weapons/CProjectileWeapon.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/vector.hpp"

class CGenDescription;
class CLight;
class CProjectileTouchResult {
public:
  CProjectileTouchResult(const TUniqueId& id, const rstl::optional_object< CRayCastResult >& result)
  : mId(id), mResult(result) {}
  TUniqueId GetActorId() const { return mId; }
  bool HasRayCastResult() const { return mResult.valid(); }
  const CRayCastResult& GetRayCastResult() const { return *mResult; }

private:
  TUniqueId mId;
  rstl::optional_object< CRayCastResult > mResult;
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
  const CProjectileWeapon& GetProjectile() const { return mProjectile; }
  CProjectileWeapon& Projectile() { return mProjectile; }
  CVector3f GetVelocity() const { return GetTranslation() - mPreviousPos; }
  const CVector3f& GetPreviousPos() const { return mPreviousPos; }
  TUniqueId GetProjectileLightId() const { return mProjectileLight; }
  TUniqueId GetHomingTargetId() const { return mHomingTargetId; }
  void SetHomingTargetId(TUniqueId id) { mHomingTargetId = id; }
  TUniqueId GetHitProjectileOwner() const { return mHitProjectileOwner; }
  void SetHitProjectileOwner(TUniqueId id) { mHitProjectileOwner = id; }

  void SetLastObjectId(TUniqueId id) { mLastResolvedObj = id; }

  void SetMinHomingDistance(float distance) { mMinHomingDist = distance; }

  bool GetWeaponActive() const { return mActive; }
  void DeleteProjectileLight(CStateManager&);

#if VERSION >= VERSION_GM8E_02
  bool GetUnkPalFlag() const { return mUnkPalFlag; }
#endif

  void SetUnkPalFlag(bool active) {
#if VERSION >= VERSION_GM8E_02
    mUnkPalFlag = active;
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
  rstl::optional_object< TLockedToken< CGenDescription > > mVisorParticle;
  ushort mVisorSfx;
  CProjectileWeapon mProjectile;
  CVector3f mPreviousPos;
  float mProjExtent;
  float mHomingDt;
  double mTargetHomingTime;
  double mCurHomingTime;
  TUniqueId mHomingTargetId;
  TUniqueId mLastResolvedObj;
  TUniqueId mHitProjectileOwner;
  TUniqueId mPendingDamagee;
  TUniqueId mProjectileLight;
  CAssetId mWpscId;
  rstl::vector< CProjectileTouchResult > mTouchResults;
  float mMinHomingDist;
  bool mActive : 1;
  bool mStartedUnderwater : 1;
  bool mWaterUpdate : 1;
  bool mInWater : 1;
  bool mSendProjectileCollideMsg : 1;
#if VERSION >= VERSION_GM8E_02
  bool mUnkPalFlag : 1;
#endif
};
CHECK_SIZEOF(CGameProjectile, (VERSION >= VERSION_GM8P_00 ? 0x310
                               : VERSION >= VERSION_GM8E_02                             ? 0x2f8
                                                                                        : 0x2e8))

#endif // _CGAMEPROJECTILE
