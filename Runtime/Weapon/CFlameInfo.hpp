#ifndef __URDE_CFLAMEINFO_HPP__
#define __URDE_CFLAMEINFO_HPP__

#include "Weapon/CGameProjectile.hpp"

namespace urde
{
class CFlameInfo
{
    ResId x8_flameFxId;
public:
    CFlameInfo(s32, u32, s32, s32, float, float, float);

    void GetAttributes() const;
    void GetLength() const;
    ResId GetFlameFxId() const { return x8_flameFxId; }
};
}
#endif // __URDE_CFLAMEINFO_HPP__
