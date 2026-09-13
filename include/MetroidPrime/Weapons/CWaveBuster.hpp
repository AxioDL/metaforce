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
  bool IsFiring() const { return x3d0_24_firing; }

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

  CTransform4f x2e8_originalXf;
  CVector3f x318_bezierB;
  CVector3f x324_bezierC;
  CVector3f x330_previousBezierC;
  CVector3f x33c_homingTargetPoint;
  CVector3f x348_targetPoint;
  TLockedToken< CSwooshDescription > x354_busterSwoosh1;
  TLockedToken< CSwooshDescription > x360_busterSwoosh2;
  TLockedToken< CGenDescription > x36c_busterSparks;
  TLockedToken< CGenDescription > x378_busterLight;
  rstl::single_ptr< CParticleSwoosh > x384_busterSwoosh1Gen;
  rstl::single_ptr< CParticleSwoosh > x388_busterSwoosh2Gen;
  rstl::single_ptr< CElementGen > x38c_busterSparksGen;
  rstl::single_ptr< CElementGen > x390_busterLightGen;
  mutable CRandom16 x394_rand;
  float x398_spiralOffset;
  float x39c_;
  float x3a0_bezierBlend;
  float x3a4_sourceAngleRate;
  float x3a8_sourceAngleTimer;
  float x3ac_sourceAngle;
  float x3b0_sourceRadius;
  float x3b4_sourceRadiusRate;
  float x3b8_sourceRadiusTimer;
  float x3bc_targetAngleRate;
  float x3c0_targetAngleTimer;
  float x3c4_targetAngle;
  float x3c8_innerSwooshColorT;
  uint x3cc_innerSwooshColorIdx;
  bool x3d0_24_firing : 1;
  bool x3d0_25_seeking : 1;
  bool x3d0_26_trackingTarget : 1;
  bool x3d0_27_collided : 1;
  bool x3d0_28_collidedWithWorld : 1;
};
CHECK_SIZEOF(CWaveBuster, 0x3d8)

#endif // _CWAVEBUSTER
