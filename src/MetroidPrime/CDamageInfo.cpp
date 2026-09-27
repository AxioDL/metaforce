#include "MetroidPrime/CDamageInfo.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"

CDamageInfo::CDamageInfo(CInputStream& in)
: mWeaponMode(CWeaponMode::Invalid()), mNoImmunity(false) {
  in.ReadLong();
  mWeaponMode = CWeaponMode(EWeaponType(in.ReadLong()));
  mDamage = in.ReadFloat();
  mRadiusDamage = mDamage;
  mRadius = in.ReadFloat();
  mKnockback = in.ReadFloat();
}

float CDamageInfo::GetDamage(const CDamageVulnerability& dVuln) const {
  EVulnerability vuln = dVuln.GetVulnerability(mWeaponMode, CDamageVulnerability::kRD_No);
  if (vuln == kVN_Deflect)
    return 0.f;
  else if (vuln == kVN_Weak)
    return 2.f * mDamage;

  return mDamage;
}

float CDamageInfo::GetRadiusDamage(const CDamageVulnerability& dVuln) const {
  EVulnerability vuln = dVuln.GetVulnerability(mWeaponMode, CDamageVulnerability::kRD_No);
  if (vuln == kVN_Deflect) {
    return 0.f;
  }
  if (vuln == kVN_Weak) {
    return 2.f * mRadiusDamage;
  }

  return mRadiusDamage;
}

CDamageInfo::CDamageInfo(const CDamageInfo& other, float dt)
: mWeaponMode(other.mWeaponMode)
, mDamage(other.mDamage * (60 * dt))
, mRadiusDamage(mDamage)
, mRadius(other.mRadius)
, mKnockback(other.mKnockback)
, mNoImmunity(true) {}

CDamageInfo CDamageInfo::MakeScaledForTime(const float dt) const {
  return CDamageInfo(mWeaponMode, mDamage * (60.f * dt), mRadius, mKnockback, true);
}
