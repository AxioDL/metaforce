#ifndef __URDE_CWAVEBEAM_HPP__
#define __URDE_CWAVEBEAM_HPP__

#include "CGunWeapon.hpp"

namespace urde
{

class CWaveBeam : public CGunWeapon
{
    TCachedToken<CWeaponDescription> x21c_waveBeam;
    TCachedToken<CElectricDescription> x228_wave2nd1;
    TCachedToken<CElectricDescription> x234_wave2nd2;
    TCachedToken<CGenDescription> x240_wave2nd3;
    float x24c_ = 0.f;
    u32 x250_ = 0;
    u32 x254_ = 0;
    bool x258_24 : 1;
    bool x258_25 : 1;
public:
    CWaveBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId,
              EMaterialTypes playerMaterial, const zeus::CVector3f& scale);
};

}

#endif // __URDE_CWAVEBEAM_HPP__
