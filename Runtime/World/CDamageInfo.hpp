#ifndef __URDE_CDAMAGEINFO_HPP__
#define __URDE_CDAMAGEINFO_HPP__

#include "RetroTypes.hpp"
#include "CWeaponMgr.hpp"

namespace urde
{

class CDamageVulnerability;
class CDamageInfo
{
    EWeaponType x0_type = EWeaponType::None;
    union
    {
        struct
        {
            bool x4_24_ : 1;
            bool x4_25_ : 1;
            bool x4_26_ : 1;
        };
        u8 _dummy = 0;
    };
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
        x0_type = EWeaponType(in.readUint32Big());
        x8_damage = in.readFloatBig();
        xc_radiusDamage = x8_damage;
        x10_radius = in.readFloatBig();
        x14_knockback = in.readFloatBig();
    }

    float GetRadiusDamage() const { return xc_radiusDamage; }
    float GetRadiusDamage(const CDamageVulnerability& dVuln);
};

}

#endif // __URDE_CDAMAGEINFO_HPP__
