#ifndef __URDE_CDAMAGEINFO_HPP__
#define __URDE_CDAMAGEINFO_HPP__

#include "RetroTypes.hpp"
#include "Weapon/CWeaponMgr.hpp"
#include "Weapon/CWeaponMode.hpp"

namespace urde
{

class CDamageVulnerability;
class CDamageInfo
{
    CWeaponMode x0_weaponMode;
    float x8_damage;
    float xc_radiusDamage;
    float x10_radius;
    float x14_knockback;
    bool x18_ = false;
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
    CDamageInfo(const CWeaponMode&, float damage, float radius, float knockback);
    CDamageInfo(const CDamageInfo& other) = default;

    const CWeaponMode& GetWeaponMode() const { return x0_weaponMode; }
    float GetRadius() const { return x10_radius; }
    void SetRadius(float r) { x10_radius = r; }
    float GetKnockBackPower() const { return x14_knockback; }
    float GetDamage() const { return x8_damage; }
    void GetDamage(const CDamageVulnerability& dVuln);
    float GetRadiusDamage() const { return xc_radiusDamage; }
    float GetRadiusDamage(const CDamageVulnerability& dVuln);
};

}

#endif // __URDE_CDAMAGEINFO_HPP__
