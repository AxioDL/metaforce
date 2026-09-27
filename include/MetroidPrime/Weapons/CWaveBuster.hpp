#ifndef _CWAVEBUSTER
#define _CWAVEBUSTER

#include "MetroidPrime/Weapons/CGameProjectile.hpp"

#include "Kyoto/CRandom16.hpp"
#include "rstl/single_ptr.hpp"

class CParticleSwoosh;
class CSwooshDescription;

class CWaveBuster : public CGameProjectile {
public:
  // CEntity
  ~CWaveBuster() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;

  // CActor
  void AddToRenderer(const CFrustumPlanes& planes, const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  CWaveBuster(const TToken< CWeaponDescription >& desc, EWeaponType type, const CTransform4f& xf,
              EMaterialTypes matType, const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid,
              TUniqueId owner, TUniqueId homingTarget, uint attribs);
  void SetNewTarget(TUniqueId uid, CStateManager& mgr);
  void UpdateFx(const CTransform4f& xf, float dt, CStateManager& mgr);
  void ResetBeam(bool deactivate);
  bool IsFiring() const { return mFiring; }

private:
  float GetViewAngleToTarget(CVector3f& direction, const CActor& actor) const;
  bool UpdateBeamFrame(CStateManager& mgr, float dt);
  void UpdateTargetDamage(float dt, CStateManager& mgr);
  CRayCastResult SeekTarget(TUniqueId& uid, float dt, CStateManager& mgr);
  void UpdateTargetSeek(float dt, CStateManager& mgr);
  void RenderElectricSpiral() const;
  void RenderSwooshes() const;
  CRayCastResult CollideWithWorld(TUniqueId& uid, const CVector3f& pos, const CVector3f& dir,
                                  CStateManager& mgr, float dt);
  void CollideWithObject(const CStateManager& mgr, TUniqueId& physicsId, TUniqueId& actorId,
                         const CVector3f& pos, const CVector3f& dir, float length,
                         CRayCastResult& physicsResult, CRayCastResult& actorResult);
  bool IsNearest(const TUniqueId& otherId, const CRayCastResult& result,
                 const CRayCastResult& otherResult, const CRayCastResult& staticResult, float dt,
                 CStateManager& mgr);

  CTransform4f mOriginalXf;
  CVector3f mBezierB;
  CVector3f mBezierC;
  CVector3f mPreviousBezierC;
  CVector3f mHomingTargetPoint;
  CVector3f mTargetPoint;
  TLockedToken< CSwooshDescription > mBusterSwoosh1;
  TLockedToken< CSwooshDescription > mBusterSwoosh2;
  TLockedToken< CGenDescription > mBusterSparks;
  TLockedToken< CGenDescription > mBusterLight;
  rstl::single_ptr< CParticleSwoosh > mBusterSwoosh1Gen;
  rstl::single_ptr< CParticleSwoosh > mBusterSwoosh2Gen;
  rstl::single_ptr< CElementGen > mBusterSparksGen;
  rstl::single_ptr< CElementGen > mBusterLightGen;
  mutable CRandom16 mRand;
  float mSpiralOffset;
  float x39c_;
  float mBezierBlend;
  float mSourceAngleRate;
  float mSourceAngleTimer;
  float mSourceAngle;
  float mSourceRadius;
  float mSourceRadiusRate;
  float mSourceRadiusTimer;
  float mTargetAngleRate;
  float mTargetAngleTimer;
  float mTargetAngle;
  float mInnerSwooshColorT;
  uint mInnerSwooshColorIdx;
  bool mFiring : 1;
  bool mSeeking : 1;
  bool mTrackingTarget : 1;
  bool mCollided : 1;
  bool mCollidedWithWorld : 1;
};
CHECK_SIZEOF(CWaveBuster, (VERSION >= VERSION_GM8P_00 ? 0x400
                           : VERSION >= VERSION_GM8E_02                             ? 0x3e8
                                                                                    : 0x3d8))

#endif // _CWAVEBUSTER
