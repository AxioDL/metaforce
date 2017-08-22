#ifndef __URDE_CICEBEAM_HPP__
#define __URDE_CICEBEAM_HPP__

#include "CGunWeapon.hpp"

namespace urde
{

class CIceBeam : public CGunWeapon
{
    TCachedToken<CGenDescription> x21c_iceSmoke;
    TCachedToken<CGenDescription> x228_ice2nd1;
    TCachedToken<CGenDescription> x234_ice2nd2;
    u32 x240_ = 0;
    u32 x244_ = 0;
    bool x248_24 : 1;
    bool x248_25 : 1;
public:
    CIceBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId,
             EMaterialTypes playerMaterial, const zeus::CVector3f& scale);
};

}

#endif // __URDE_CICEBEAM_HPP__
