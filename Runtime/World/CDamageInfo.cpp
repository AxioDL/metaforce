#include "Runtime/World/CDamageInfo.hpp"

#include "Runtime/World/CDamageVulnerability.hpp"

#include "DataSpec/DNACommon/Tweaks/ITweakPlayerGun.hpp"

namespace urde {

CDamageInfo::CDamageInfo(const DataSpec::SShotParam& other)
: x0_weaponMode(CWeaponMode(EWeaponType(other.weaponType), other.charged, other.combo, other.instaKill))
, x8_damage(other.damage)
, xc_radiusDamage(other.radiusDamage)
, x10_radius(other.radius)
, x14_knockback(other.knockback)
, x18_noImmunity(other.noImmunity) {}

CDamageInfo& CDamageInfo::operator=(const DataSpec::SShotParam& other) {
  x0_weaponMode = CWeaponMode(EWeaponType(other.weaponType), other.charged, other.combo, other.instaKill);
  x8_damage = other.damage;
  xc_radiusDamage = other.radiusDamage;
  x10_radius = other.radius;
  x14_knockback = other.knockback;
  x18_noImmunity = other.noImmunity;
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
  x18_noImmunity = true;
}
} // namespace urde
