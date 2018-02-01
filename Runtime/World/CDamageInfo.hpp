#ifndef __URDE_CDAMAGEINFO_HPP__
#define __URDE_CDAMAGEINFO_HPP__

#include "RetroTypes.hpp"
#include "Weapon/CWeaponMgr.hpp"
#include "Weapon/CWeaponMode.hpp"

namespace DataSpec
{
struct SShotParam;
}

namespace urde
{
class CDamageVulnerability;
class CDamageInfo
{
    CWeaponMode x0_weaponMode;
    float x8_damage = 0.f;
    float xc_radiusDamage = 0.f;
    float x10_radius = 0.f;
    float x14_knockback = 0.f;
    bool x18_noImmunity = false;

public:
    CDamageInfo() = default;
    CDamageInfo(CInputStream& in)
    {
        in.readUint32Big();
        x0_weaponMode = CWeaponMode(EWeaponType(in.readUint32Big()));
        x8_damage = in.readFloatBig();
        xc_radiusDamage = x8_damage;
        x10_radius = in.readFloatBig();
        x14_knockback = in.readFloatBig();
    }
    CDamageInfo(const CWeaponMode& mode, float damage, float radius, float knockback)
    : x0_weaponMode(mode), x8_damage(damage), xc_radiusDamage(damage), x10_radius(radius), x14_knockback(knockback)
    {
    }

    CDamageInfo(const CDamageInfo& other) = default;
    CDamageInfo(const DataSpec::SShotParam& other);
    CDamageInfo& operator=(const DataSpec::SShotParam& other);

    const CWeaponMode& GetWeaponMode() const { return x0_weaponMode; }
    void SetWeaponMode(const CWeaponMode& mode) { x0_weaponMode = mode; }
    float GetRadius() const { return x10_radius; }
    void SetRadius(float r) { x10_radius = r; }
    float GetKnockBackPower() const { return x14_knockback; }
    void SetKnockBackPower(float k) { x14_knockback = k; }
    float GetDamage() const { return x8_damage; }
    void SetDamage(float d) { x8_damage = d; }
    float GetDamage(const CDamageVulnerability& dVuln) const;
    float GetRadiusDamage() const { return xc_radiusDamage; }
    float GetRadiusDamage(const CDamageVulnerability& dVuln) const;
    bool NoImmunity() const { return x18_noImmunity; }
    void SetNoImmunity(bool b) { x18_noImmunity = b; }
    void MultiplyDamage(float m)
    {
        x8_damage *= m;
        xc_radiusDamage *= m;
        x14_knockback *= m;
    }
};
}

#endif // __URDE_CDAMAGEINFO_HPP__
