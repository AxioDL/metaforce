#ifndef __URDE_CWEAPONMODE_HPP__
#define __URDE_CWEAPONMODE_HPP__

#include "CWeaponMgr.hpp"

namespace urde
{
class CWeaponMode
{
    EWeaponType x0_weaponType;
    union
    {
        struct
        {
            bool x4_24_ : 1;
            bool x4_25_ : 1;
            bool x4_26_instantKill : 1;
        };
        u8 dummy = 0;
    };
public:
    CWeaponMode(EWeaponType, bool, bool);
    EWeaponType GetType() const;


    bool IsCharged() const;
    bool IsComboed() const;
    bool IsInstantKill() const;

    static CWeaponMode Invalid();
    static CWeaponMode Phazon();
    static CWeaponMode Plasma();
    static CWeaponMode Wave();
    static CWeaponMode BoostBall();
    static CWeaponMode Ice();
    static CWeaponMode Power();
    static CWeaponMode Bomb();
};
}
#endif // __URDE_CWEAPONMODE_HPP__
