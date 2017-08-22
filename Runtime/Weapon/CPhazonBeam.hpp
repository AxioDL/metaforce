#ifndef __URDE_CPHAZONBEAM_HPP__
#define __URDE_CPHAZONBEAM_HPP__

#include "CGunWeapon.hpp"

namespace urde
{

class CPhazonBeam : public CGunWeapon
{
    TCachedToken<CModel> x21c_phazonVeins;
    TCachedToken<CGenDescription> x228_phazon2nd1;
    u32 x234_ = 0;
    zeus::CAABox x238_;
    zeus::CAABox x250_;
    float x268_ = 0.f;
    float x26c_ = 0.f;
    float x270_ = 1.f;
    bool x274_24 : 1;
    bool x274_25 : 1;
    bool x274_26 : 1;
    bool x274_27 : 1;
    float x278_ = 1.f / 3.f;
public:
    CPhazonBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId,
                EMaterialTypes playerMaterial, const zeus::CVector3f& scale);
};

}

#endif // __URDE_CPHAZONBEAM_HPP__
