#ifndef __URDE_CPOWERBEAM_HPP__
#define __URDE_CPOWERBEAM_HPP__

#include "CGunWeapon.hpp"

namespace urde
{

class CPowerBeam : public CGunWeapon
{
    TCachedToken<CGenDescription> x21c_shotSmoke;
    TCachedToken<CGenDescription> x228_power2nd1;
    std::unique_ptr<CElementGen> x234_shotSmokeGen;
    float x23c_ = 0.f;
    u32 x240_ = 0;
    bool x244_24 : 1;
    bool x244_25 : 1;
public:
    CPowerBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId,
               EMaterialTypes playerMaterial, const zeus::CVector3f& scale);
};

}

#endif // __URDE_CPOWERBEAM_HPP__
