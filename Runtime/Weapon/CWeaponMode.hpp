#ifndef __URDE_CWEAPONMODE_HPP__
#define __URDE_CWEAPONMODE_HPP__

#include "WeaponCommon.hpp"

namespace urde
{
class CWeaponMode
{
    EWeaponType x0_weaponType = EWeaponType::None;
    bool x4_24_charged : 1;
    bool x4_25_comboed : 1;
    bool x4_26_instantKill : 1;
public:
    CWeaponMode() = default;
    CWeaponMode(EWeaponType, bool charged = false, bool comboed = false, bool instaKill = false);
    EWeaponType GetType() const;


    bool IsCharged() const { return x4_24_charged; }
    bool IsComboed() const { return x4_25_comboed; }
    bool IsInstantKill() const { return x4_26_instantKill; }

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
