#pragma once

#include "Runtime/Tweaks/ITweak.hpp"

namespace metaforce {
/* Same as CDamageInfo */
struct SShotParam {
  u32 x0_weaponType = -1;
  bool x4_24_charged : 1 = false;
  bool x4_25_combo : 1 = false;
  bool x4_26_instaKill : 1 = false;
  float x8_damage = 0.f;
  float xc_radiusDamage = 0.f;
  float x10_radius = 0.f;
  float x14_knockback = 0.f;
  bool x18_24_noImmunity : 1 = false;
  SShotParam() = default;
  explicit SShotParam(CInputStream& in);
};

struct SComboShotParam : SShotParam {
  SComboShotParam() { x4_25_combo = true; }
  explicit SComboShotParam(CInputStream& in) : SShotParam(in) { x4_25_combo = true; }
};

struct SChargedShotParam : SShotParam {
  SChargedShotParam() { x4_24_charged = true; }
  explicit SChargedShotParam(CInputStream& in) : SShotParam(in) { x4_24_charged = true; }
};

struct SWeaponInfo {
  float x0_coolDown = 0.1f;
  SShotParam x4_normal;
  SChargedShotParam x20_charged;
  SWeaponInfo() = default;
  explicit SWeaponInfo(CInputStream& in);
};

namespace Tweaks {
struct ITweakPlayerGun : ITweak {
  virtual float GetUpLookAngle() const = 0;
  virtual float GetDownLookAngle() const = 0;
  virtual float GetVerticalSpread() const = 0;
  virtual float GetHorizontalSpread() const = 0;
  virtual float GetHighVerticalSpread() const = 0;
  virtual float GetHighHorizontalSpread() const = 0;
  virtual float GetLowVerticalSpread() const = 0;
  virtual float GetLowHorizontalSpread() const = 0;
  virtual float GetAimVerticalSpeed() const = 0;   // x24
  virtual float GetAimHorizontalSpeed() const = 0; // x28
  virtual float GetBombFuseTime() const = 0;       // x2c
  virtual float GetBombDropDelayTime() const = 0;  // x30
  virtual float GetHoloHoldTime() const = 0;       // x34
  virtual float GetGunTransformTime() const = 0;   // x38
  virtual float GetGunHolsterTime() const = 0;
  virtual float GetGunNotFiringTime() const = 0;
  virtual float GetFixedVerticalAim() const = 0;
  virtual float GetGunExtendDistance() const = 0;
  virtual const zeus::CVector3f& GetGunPosition() const = 0;
  virtual const zeus::CVector3f& GetGrapplingArmPosition() const = 0;
  virtual float GetRichochetDamage(u32) const = 0;
  virtual const SWeaponInfo& GetBeamInfo(s32 beam) const = 0;
  virtual const SComboShotParam& GetComboShotInfo(s32 beam) const = 0;
  virtual const SShotParam& GetBombInfo() const = 0;
  virtual const SShotParam& GetPowerBombInfo() const = 0;
};
} // namespace Tweaks
} // namespace metaforce
