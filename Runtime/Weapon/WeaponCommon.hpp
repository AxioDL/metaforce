#ifndef __URDE_WEAPONCOMMON_HPP__
#define __URDE_WEAPONCOMMON_HPP__

#include "RetroTypes.hpp"
#include "DataSpec/DNACommon/Tweaks/ITweakPlayerGun.hpp"

namespace urde
{
enum class EWeaponType
{
    None = -1,
    Power = 0,
    Ice = 1,
    Wave = 2,
    Plasma = 3,
    Bomb = 4,
    PowerBomb = 5,
    Missile = 6,
    BoostBall = 7,
    Phazon = 8,
    AI = 9,
    PoisonWater = 10,
    Lava = 11,
    Hot = 12,
    Unused1 = 13,
    Unused2 = 14
};
}
#endif // __URDE_WEAPONCOMMON_HPP__
