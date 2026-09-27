#ifndef _CELECTRICBEAMPROJECTILE
#define _CELECTRICBEAMPROJECTILE

#include "MetroidPrime/Weapons/CBeamProjectile.hpp"

class CParticleElectric;
class CElectricDescription;

struct SElectricBeamInfo {
  SElectricBeamInfo(const TToken< CElectricDescription >& tok, float maxLength, float radius,
                    float travelSpeed, CAssetId particle, float fadeSpeed, float damageInterval)
  : mElectricDescription(tok)
  , mMaxLength(maxLength)
  , mRadius(radius)
  , mTravelSpeed(travelSpeed)
  , mParticleId(particle)
  , mFadeSpeed(fadeSpeed)
  , mDamageInterval(damageInterval) {}
  TToken< CElectricDescription > mElectricDescription;
  float mMaxLength;
  float mRadius;
  float mTravelSpeed;
  CAssetId mParticleId;
  float mFadeSpeed;
  float mDamageInterval;
};

class CElectricBeamProjectile : public CBeamProjectile {
public:
  CElectricBeamProjectile(const TToken< CWeaponDescription >&, EWeaponType,
                          const SElectricBeamInfo&, const CTransform4f&, EMaterialTypes,
                          const CDamageInfo&, TUniqueId, TAreaId, TUniqueId, EProjectileAttrib);

  // CEntity
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;

  // CActor
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  void Touch(CActor&, CStateManager&) override;

  // CBeamProjectile
  void UpdateFx(const CTransform4f&, float, CStateManager&) override;
  void ResetBeam(CStateManager&, bool) override;
  void Fire(const CTransform4f&, CStateManager&, bool) override;

private:
  rstl::single_ptr< CParticleElectric > mElectric;
  TLockedToken< CGenDescription > mGenDescription;
  rstl::single_ptr< CElementGen > mElementGen;
  float mFadeSpeed;
  float mIntensity;
  float mDamageTimer;
  float mDamageInterval;
  bool x48c_;
};

#endif // _CELECTRICBEAMPROJECTILE
