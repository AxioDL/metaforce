#include "CDamageInfo.hpp"
#include "CDamageVulnerability.hpp"
#include "DataSpec/DNACommon/Tweaks/ITweakPlayerGun.hpp"

namespace urde
{

float CDamageInfo::GetRadiusDamage(const CDamageVulnerability& dVuln) { return 0.f; }

CDamageInfo::CDamageInfo(const DataSpec::SShotParam& other)
: x0_weaponMode(CWeaponMode(EWeaponType(other.weaponType), other.Charged(), other.Comboed(), other.InstaKill()))
, x8_damage(other.damage)
, xc_radiusDamage(other.radiusDamage)
, x10_radius(other.radius)
, x14_knockback(other.knockback)
{
}

CDamageInfo& CDamageInfo::operator=(const DataSpec::SShotParam& other)
{
    x0_weaponMode = CWeaponMode(EWeaponType(other.weaponType), other.Charged(), other.Comboed(), other.InstaKill());
    x8_damage = other.damage;
    xc_radiusDamage = x8_damage;
    x10_radius = other.radius;
    x14_knockback = other.knockback;
    x18_ = false;
    return *this;
}
}
