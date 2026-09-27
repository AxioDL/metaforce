#ifndef _CTWEAKPLAYERGUN
#define _CTWEAKPLAYERGUN

#include "types.h"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Tweaks/ITweakObject.hpp"

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TOneStatic.hpp"

#include "rstl/reserved_vector.hpp"

struct SWeaponInfo {
  float mCoolDown;
  CDamageInfo mNormal;
  CDamageInfo mCharged;

  SWeaponInfo() {}
};
CHECK_SIZEOF(SWeaponInfo, 0x3c)

class CTweakPlayerGun : public ITweakObject, public TOneStatic< CTweakPlayerGun > {
public:
  CTweakPlayerGun(CInputStream& in);
  ~CTweakPlayerGun() override {}

  float GetUpLookAngle() const { return mUpLookAngle; }
  float GetDownLookAngle() const { return mDownLookAngle; }
  float GetVerticalSpread() const { return mVerticalSpread; }
  float GetHorizontalSpread() const { return mHorizontalSpread; }
  float GetHighVerticalSpread() const { return mHighVerticalSpread; }
  float GetHighHorizontalSpread() const { return mHighHorizontalSpread; }
  float GetLowVerticalSpread() const { return mLowVerticalSpread; }
  float GetLowHorizontalSpread() const { return mLowHorizontalSpread; }
  float GetAimVerticalSpeed() const { return mAimVerticalSpeed; }
  float GetAimHorizontalSpeed() const { return mAimHorizontalSpeed; }
  float GetBombFuseTime() const { return mBombFuseTime; }
  float GetBombDropDelayTime() const { return mBombDropDelayTime; }
  float GetHoloHoldTime() const { return mHoloHoldTime; }
  float GetGunTransformTime() const { return mGunTransformTime; }
  float GetGunHolsterTime() const { return mGunHolsterTime; }
  float GetGunNotFiringTime() const { return mGunNotFiringTime; }
  float GetFixedVerticalAim() const { return mFixedVerticalAim; }
  float GetGunExtendDistance() const { return mGunExtendDistance; }
  const CVector3f& GetGunPosition() const { return mGunPosition; }
  const CVector3f& GetGrapplingArmPosition() const { return mGrapplingArmPosition; }

  const SWeaponInfo& GetBeamInfo(int beam) const;
  float GetRichochetDamage(EWeaponType type) const;

  const CDamageInfo& GetComboDamage(int beam) const { return mCombos[beam]; }
  const CDamageInfo& GetMissleDamage() const { return mMissile; }

  const CDamageInfo& GetBombInfo() const { return mBomb; }

private:
  void InitBeamData(CInputStream& in);
  void InitRicochetDamageFactor(CInputStream& in);

public:
  // private:
  float mUpLookAngle;
  float mDownLookAngle;
  float mVerticalSpread;
  float mHorizontalSpread;
  float mHighVerticalSpread;
  float mHighHorizontalSpread;
  float mLowVerticalSpread;
  float mLowHorizontalSpread;
  float mAimVerticalSpeed;
  float mAimHorizontalSpeed;
  float mBombFuseTime;
  float mBombDropDelayTime;
  float mHoloHoldTime;
  float mGunTransformTime;
  float mGunHolsterTime;
  float mGunNotFiringTime;
  float mFixedVerticalAim;
  float mGunExtendDistance;
  CVector3f mGunPosition;
  CVector3f x58_;
  CVector3f mGrapplingArmPosition;
  CDamageInfo mBomb;
  CDamageInfo mPowerBomb;
  SWeaponInfo mPowerBeam;
  SWeaponInfo mIceBeam;
  SWeaponInfo mWaveBeam;
  SWeaponInfo mPlasmaBeam;
  SWeaponInfo mPhazonBeam;
  CDamageInfo mMissile;
  rstl::reserved_vector< CDamageInfo, 5 > mCombos;
  rstl::reserved_vector< float, 5 > mRicochetData;
};
CHECK_SIZEOF(CTweakPlayerGun, 0x298)

extern CTweakPlayerGun* gpTweakPlayerGun;

#endif // _CTWEAKPLAYERGUN
