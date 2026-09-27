#ifndef _CDAMAGEINFO
#define _CDAMAGEINFO

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"

class CDamageVulnerability;

class CDamageInfo {
public:
  CDamageInfo()
  : mWeaponMode(CWeaponMode::Invalid())
  , mDamage(0.f)
  , mRadiusDamage(mDamage)
  , mRadius(0.f)
  , mKnockback(0.f)
  , mNoImmunity(false) {}

  CDamageInfo(const CWeaponMode& mode, const float damage, const float radius,
              const float knockback, const bool noImmunity = false)
  : mWeaponMode(mode)
  , mDamage(damage)
  , mRadiusDamage(mDamage)
  , mRadius(radius)
  , mKnockback(knockback)
  , mNoImmunity(noImmunity) {}

  // HACK: This constructor exists to fix CWeapon::Think writing xc_radiusDamage as a double
  CDamageInfo(const CWeaponMode& mode, float damage, double radiusDamage, float radius,
              float knockback, bool noImmunity = false)
  : mWeaponMode(mode)
  , mDamage(damage)
  , mRadiusDamage(radiusDamage)
  , mRadius(radius)
  , mKnockback(knockback)
  , mNoImmunity(noImmunity) {}

  CDamageInfo(CInputStream& in);
  CDamageInfo(const CDamageInfo&, float);

  const CWeaponMode& GetWeaponMode() const { return mWeaponMode; }
  void SetWeaponMode(const CWeaponMode& mode) { mWeaponMode = mode; }
  float GetRadius() const { return mRadius; }
  void SetRadius(float r) { mRadius = r; }
  float GetKnockBackPower() const { return mKnockback; }
  void SetKnockBackPower(float k) { mKnockback = k; }
  float GetDamage() const { return mDamage; }
  void SetDamage(float d) { mDamage = d; }
  bool HasNoDamage() const { return mDamage <= 0.0f; }
  float GetDamage(const CDamageVulnerability& dVuln) const;
  float GetRadiusDamage() const { return mRadiusDamage; }
  void SetRadiusDamage(float r) { mRadiusDamage = r; }
  float GetRadiusDamage(const CDamageVulnerability& dVuln) const;
  bool NoImmunity() const { return mNoImmunity; }
  void SetNoImmunity(bool b) { mNoImmunity = b; }
  void MultiplyDamage(const float m) {
    mDamage = m * mDamage;
    mRadiusDamage = m * mRadiusDamage;
    mKnockback = m * mKnockback;
  }
  CDamageInfo& MultiplyDamageAndRadius(float m) {
    mDamage = m * mDamage;
    mRadiusDamage = m * mRadiusDamage;
    mRadius = m * mRadius;
    mKnockback = m * mKnockback;
    return *this;
  }

  CDamageInfo MakeScaledForTime(const float dt) const;

private:
  CWeaponMode mWeaponMode;
  float mDamage;
  float mRadiusDamage;
  float mRadius;
  float mKnockback;
  bool mNoImmunity : 1;
};

#endif // _CDAMAGEINFO
