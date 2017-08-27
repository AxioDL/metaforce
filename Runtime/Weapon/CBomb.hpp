#ifndef __URDE_CBOMB_HPP__
#define __URDE_CBOMB_HPP__

#include "CWeapon.hpp"
#include "CToken.hpp"

namespace urde
{

class CBomb : public CWeapon
{
public:
    CBomb(const TToken<CGenDescription>& particle1, const TToken<CGenDescription>& particle2,
          TUniqueId uid, TAreaId aid, TUniqueId playerId, float f1,
          const zeus::CTransform& xf, const CDamageInfo& dInfo);
};

}

#endif // __URDE_CBOMB_HPP__
