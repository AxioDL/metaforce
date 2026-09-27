#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

static inline CDamageInfo read_damageInfo(CInputStream& in, bool charged, bool combo) {
  const EWeaponType type = static_cast< EWeaponType >(in.ReadLong());
  const float damage = in.ReadFloat();
  const float radiusDamage = in.ReadFloat();
  const float radius = in.ReadFloat();
  const float knockback = in.ReadFloat();
  CDamageInfo info(CWeaponMode(type, charged, combo), damage, radius, knockback);
  info.SetRadiusDamage(radiusDamage);
  return info;
}

inline void CTweakPlayerGun::InitRicochetDamageFactor(CInputStream& in) {
  for (int i = 0; i < 5; ++i) {
    const float factor = in.ReadFloat();
    mRicochetData[i] = factor;
  }
}

CTweakPlayerGun::CTweakPlayerGun(CInputStream& in)
: mUpLookAngle(in.ReadFloat())
, mDownLookAngle(in.ReadFloat())
, mVerticalSpread(in.ReadFloat())
, mHorizontalSpread(in.ReadFloat())
, mHighVerticalSpread(in.ReadFloat())
, mHighHorizontalSpread(in.ReadFloat())
, mLowVerticalSpread(in.ReadFloat())
, mLowHorizontalSpread(in.ReadFloat())
, mAimVerticalSpeed(in.ReadFloat())
, mAimHorizontalSpeed(in.ReadFloat())
, mBombFuseTime(in.ReadFloat())
, mBombDropDelayTime(in.ReadFloat())
, mHoloHoldTime(in.ReadFloat())
, mGunTransformTime(in.ReadFloat())
, mGunHolsterTime(in.ReadFloat())
, mGunNotFiringTime(in.ReadFloat())
, mFixedVerticalAim(in.ReadFloat() * (M_PIF / 180.f))
, mGunExtendDistance(in.ReadFloat())
, mGunPosition(in)
, x58_(in)
, mGrapplingArmPosition(in)
, mBomb(read_damageInfo(in, false, false))
, mPowerBomb(read_damageInfo(in, false, false))
, mMissile(read_damageInfo(in, false, false))
, mCombos(5, CDamageInfo())
, mRicochetData(5, 0.1f) {
  InitBeamData(in);
  for (int i = 0; i < 5; ++i) {
    mCombos[i] = read_damageInfo(in, false, true);
  }
  InitRicochetDamageFactor(in);
}

void CTweakPlayerGun::InitBeamData(CInputStream& in) {
  SWeaponInfo* beams[5] = {&mPowerBeam, &mIceBeam, &mWaveBeam, &mPlasmaBeam,
                           &mPhazonBeam};
  for (int i = 0; i < 5; ++i) {
    SWeaponInfo& beam = *beams[i];
    beam.mCoolDown = in.ReadFloat();
    beam.mNormal = read_damageInfo(in, false, false);
    beam.mCharged = read_damageInfo(in, true, false);
  }
}

const SWeaponInfo& CTweakPlayerGun::GetBeamInfo(int beam) const {
  const SWeaponInfo* beams[5] = {&mPowerBeam, &mIceBeam, &mWaveBeam, &mPlasmaBeam,
                                 &mPhazonBeam};
  if (beam < 0 || beam > 5) {
    return mPowerBeam;
  }
  return *beams[beam];
}

float CTweakPlayerGun::GetRichochetDamage(EWeaponType type) const {
  switch (type) {
  case kWT_Power:
    return mRicochetData[0];
  case kWT_Ice:
    return mRicochetData[1];
  case kWT_Wave:
    return mRicochetData[2];
  case kWT_Plasma:
    return mRicochetData[3];
  case kWT_Missile:
    return mRicochetData[4];
  case kWT_Phazon:
    return mRicochetData[5];
  default:
    return 1.f;
  }
}
