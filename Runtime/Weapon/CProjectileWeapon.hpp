#ifndef __URDE_CPROJECTILEWEAPON_HPP__
#define __URDE_CPROJECTILEWEAPON_HPP__

#include "RetroTypes.hpp"
#include "CRandom16.hpp"

namespace urde
{

class CProjectileWeapon
{
    static CRandom16 g_GlobalSeed;
public:
    static void SetGlobalSeed(u16 seed) { g_GlobalSeed.SetSeed(seed); }
};

}

#endif // __URDE_CPROJECTILEWEAPON_HPP__
