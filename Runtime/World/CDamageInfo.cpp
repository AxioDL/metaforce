#include "Runtime/World/CDamageInfo.hpp"

#include "Runtime/World/CDamageVulnerability.hpp"

namespace metaforce {

CDamageInfo::CDamageInfo(const SShotParam& other)
: x0_weaponMode(CWeaponMode(EWeaponType(other.x0_weaponType), other.x4_24_charged, other.x4_25_combo, other.x4_26_instaKill))
, x8_damage(other.x8_damage)
, xc_radiusDamage(other.xc_radiusDamage)
, x10_radius(other.x10_radius)
, x14_knockback(other.x14_knockback)
, x18_24_noImmunity(other.x18_24_noImmunity) {}

CDamageInfo& CDamageInfo::operator=(const SShotParam& other) {
  x0_weaponMode =
      CWeaponMode(EWeaponType(other.x0_weaponType), other.x4_24_charged, other.x4_25_combo, other.x4_26_instaKill);
  x8_damage = other.x8_damage;
  xc_radiusDamage = other.xc_radiusDamage;
  x10_radius = other.x10_radius;
  x14_knockback = other.x14_knockback;
  x18_24_noImmunity = other.x18_24_noImmunity;
  return *this;
}

float CDamageInfo::GetDamage(const CDamageVulnerability& dVuln) const {
  EVulnerability vuln = dVuln.GetVulnerability(x0_weaponMode, false);
  if (vuln == EVulnerability::Deflect)
    return 0.f;
  else if (vuln == EVulnerability::Weak)
    return 2.f * x8_damage;

  return x8_damage;
}

float CDamageInfo::GetRadiusDamage(const CDamageVulnerability& dVuln) const {
  EVulnerability vuln = dVuln.GetVulnerability(x0_weaponMode, false);
  if (vuln == EVulnerability::Deflect) {
    return 0.f;
  }
  if (vuln == EVulnerability::Weak) {
    return 2.f * xc_radiusDamage;
  }

  return xc_radiusDamage;
}

CDamageInfo::CDamageInfo(const CDamageInfo& other, float dt) {
  x0_weaponMode = other.x0_weaponMode;
  x8_damage = other.x8_damage * (60.f * dt);
  xc_radiusDamage = x8_damage;
  x10_radius = other.x10_radius;
  x14_knockback = other.x14_knockback;
  x18_24_noImmunity = true;
}
} // namespace metaforce
