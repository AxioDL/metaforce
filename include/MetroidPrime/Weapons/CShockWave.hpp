#ifndef _CSHOCKWAVE
#define _CSHOCKWAVE

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageInfo.hpp"

#include <Kyoto/Particles/CElectricDescription.hpp>
#include <rstl/optional_object.hpp>

class CGenDescription;
class CElementGen;

class CShockWaveInfo {
public:
  CShockWaveInfo(const CAssetId part, const CDamageInfo& dInfo, const float initialExpansionSpeed,
                 CAssetId weapon, ushort sfx, float speedIncrease = 0.f)
  : x0_(8)
  , mParticleDesc(part)
  , mDamageInfo(dInfo)
  , mInitialRadius(0.f)
  , mWidthPercent(0.5f)
  , mInitialExpansionSpeed(initialExpansionSpeed)
  , mSpeedIncrease(speedIncrease)
  , mWeaponDesc(weapon)
  , mElectrocuteSfx(sfx) {}

  const CAssetId GetParticleDescId() const { return mParticleDesc; }
  const CDamageInfo& GetDamageInfo() const { return mDamageInfo; }
  float GetInitialRadius() const { return mInitialRadius; }
  float GetWidthPercent() const { return mWidthPercent; }
  float GetInitialExpansionSpeed() const { return mInitialExpansionSpeed; }
  float GetSpeedIncrease() const { return mSpeedIncrease; }
  void SetSpeedIncrease(float speed) { mSpeedIncrease = speed; }
  CAssetId GetWeaponDescId() const { return mWeaponDesc; }
  ushort GetElectrocuteSfx() const { return mElectrocuteSfx; }

private:
  uint x0_;
  CAssetId mParticleDesc;
  CDamageInfo mDamageInfo;
  float mInitialRadius;
  float mWidthPercent;
  float mInitialExpansionSpeed;
  float mSpeedIncrease;
  CAssetId mWeaponDesc;
  ushort mElectrocuteSfx;
};

CHECK_SIZEOF(CShockWaveInfo, 0x3c)

class CShockWave : public CActor {
public:
  CShockWave(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
             const CTransform4f& xf, const TUniqueId parent, const CShockWaveInfo& data,
             const float minActiveTime, const float knockback);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  rstl::optional_object< CAABox > GetTouchBounds() const override;

  bool WasAlreadyDamaged(const TUniqueId uid) const;

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;

private:
  TUniqueId mParentId;
  CDamageInfo mDamageInfo;
  TToken< CGenDescription > mElementGenDesc;
  rstl::single_ptr< CElementGen > mElementGen;
  CShockWaveInfo mShockWaveInfo;
  float mRadius;
  float mExpansionSpeed;
  float mActiveTime;
  float mMinActiveTime;
  float mKnockBack;
  float mTimeSinceHitPlayerInAir;
  float mTimeSinceHitPlayer;
  bool mHitPlayerInAir;
  bool mHitPlayer;
  TEntityList mHitIds;
  rstl::optional_object< TToken< CElectricDescription > > mElectricDesc;
  TUniqueId mLightId;
};

CHECK_SIZEOF(CShockWave, 0x988)

#endif // _CSHOCKWAVE
