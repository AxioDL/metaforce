#ifndef __URDE_CDAMAGEINFO_HPP__
#define __URDE_CDAMAGEINFO_HPP__

#include "RetroTypes.hpp"
#include "CWeaponMgr.hpp"

namespace urde
{

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
    float x8_damage1;
    float xc_damage2;
    float x10_radius;
    float x14_knockback;
    bool x18_ = false;
public:
    CDamageInfo(CInputStream& in)
    {
        in.readUint32Big();
        x0_type = EWeaponType(in.readUint32Big());
        x8_damage1 = in.readFloatBig();
        xc_damage2 = x8_damage1;
        x10_radius = in.readFloatBig();
        x14_knockback = in.readFloatBig();
    }
};

}

#endif // __URDE_CDAMAGEINFO_HPP__
