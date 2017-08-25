#ifndef __URDE_CPLASMABEAM_HPP__
#define __URDE_CPLASMABEAM_HPP__

#include "CGunWeapon.hpp"

namespace urde
{

class CPlasmaBeam : public CGunWeapon
{
    TCachedToken<CGenDescription> x21c_plasma2nd1;
    u32 x228_ = 0;
    bool x22c_24 : 1;
    bool x22c_25 : 1;
    float x230_ = 0.f;
    float x234_ = 0.f;
    float x238_ = 0.f;
    TAreaId x23c_ = kInvalidAreaId;
public:
    CPlasmaBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId,
                EMaterialTypes playerMaterial, const zeus::CVector3f& scale);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
};

}

#endif // __URDE_CPLASMABEAM_HPP__
