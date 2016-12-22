#ifndef __URDE_CWEAPONMGR_HPP__
#define __URDE_CWEAPONMGR_HPP__

#include "RetroTypes.hpp"
#include "WeaponCommon.hpp"

namespace urde
{

class CWeaponMgr
{
    std::map<TUniqueId, rstl::reserved_vector<s32, 10>> x0_weapons;
public:
    void Add(TUniqueId, EWeaponType);
    void Remove(TUniqueId);
    void IncrCount(TUniqueId, EWeaponType);
    void DecrCount(TUniqueId, EWeaponType);
    s32 GetNumActive(TUniqueId, EWeaponType) const;
    s32 GetIndex(TUniqueId) const;
};

}

#endif // __URDE_CWEAPONMGR_HPP__
