#pragma once

#include "ITweak.hpp"
#include "zeus/CAABox.hpp"

namespace DataSpec {
/* Same as CDamageInfo */
struct SShotParam : BigDNA {
  AT_DECL_DNA_YAML
  Value<atInt32> weaponType = -1;
  bool charged : 1;
  bool combo : 1;
  bool instaKill : 1;
  Value<float> damage = 0.f;
  Value<float> radiusDamage = 0.f;
  Value<float> radius = 0.f;
  Value<float> knockback = 0.f;
  bool noImmunity : 1;
  SShotParam() {
    charged = false;
    combo = false;
    instaKill = false;
    noImmunity = false;
  }
};

struct SComboShotParam : SShotParam {
  AT_DECL_DNA_YAML
  SComboShotParam() { combo = true; }
};

struct SChargedShotParam : SShotParam {
  AT_DECL_DNA_YAML
  SChargedShotParam() { charged = true; }
};

struct SWeaponInfo : BigDNA {
  AT_DECL_DNA_YAML
  Value<float> x0_coolDown = 0.1f;
  SShotParam x4_normal;
  SChargedShotParam x20_charged;
};

struct ITweakPlayerGun : ITweak {
  AT_DECL_DNA_YAML
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
  virtual float GetRichochetDamage(atUint32) const = 0;
  virtual const SWeaponInfo& GetBeamInfo(atInt32 beam) const = 0;
  virtual const SComboShotParam& GetComboShotInfo(atInt32 beam) const = 0;
  virtual const SShotParam& GetBombInfo() const = 0;
  virtual const SShotParam& GetPowerBombInfo() const = 0;
};
} // namespace DataSpec
