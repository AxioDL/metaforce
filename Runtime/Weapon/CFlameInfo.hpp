#ifndef __URDE_CFLAMEINFO_HPP__
#define __URDE_CFLAMEINFO_HPP__

#include "Weapon/CGameProjectile.hpp"

namespace urde
{
class CFlameInfo
{
    s32 x0_;
    s32 x4_;
    ResId x8_flameFxId;
    s32 xc_;
    float x10_;
    float x18_;
    float x1c_;
public:
    CFlameInfo(s32, s32, ResId, s32, float, float, float);

    void GetAttributes() const;
    float GetLength() const;
    ResId GetFlameFxId() const { return x8_flameFxId; }
};
}
#endif // __URDE_CFLAMEINFO_HPP__
