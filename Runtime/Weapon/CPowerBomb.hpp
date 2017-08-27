#ifndef __URDE_CPOWERBOMB_HPP__
#define __URDE_CPOWERBOMB_HPP__

#include "CWeapon.hpp"
#include "CToken.hpp"

namespace urde
{

class CPowerBomb : public CWeapon
{
    float x15c_curTime = 0.f;
    float x160_ = 0.f;
public:
    CPowerBomb(const TToken<CGenDescription>& particle, TUniqueId uid, TAreaId aid,
               TUniqueId playerId, const zeus::CTransform& xf, const CDamageInfo& dInfo);
    float GetCurTime() const { return x15c_curTime; }
};

}

#endif // __URDE_CPOWERBOMB_HPP__
