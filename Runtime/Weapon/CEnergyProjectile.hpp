#ifndef __URDE_CENERGYPROJECTILE_HPP__
#define __URDE_CENERGYPROJECTILE_HPP__

#include "CGameProjectile.hpp"
#include "Camera/CCameraShakeData.hpp"

namespace urde
{

class CEnergyProjectile : public CGameProjectile
{
    u32 x2e8_ = 0;
    zeus::CVector3f x2ec_dir;
    float x2f8_mag;
    CCameraShakeData x2fc_camShake;
    union
    {
        struct
        {
            bool x3d0_24_ : 1;
            bool x3d0_25_ : 1;
            bool x3d0_26_ : 1;
            bool x3d0_27_camShakeDirty : 1;
        };
        u32 _dummy = 0;
    };
    float x3d4_ = 0.f;
public:
    CEnergyProjectile(bool active, const TToken<CWeaponDescription>& desc, EWeaponType type,
                      const zeus::CTransform& xf, EMaterialTypes materials, const CDamageInfo& damage,
                      TUniqueId uid, TAreaId aid, TUniqueId owner, TUniqueId homingTarget,
                      EProjectileAttrib attribs, bool underwater, const zeus::CVector3f& scale,
                      const rstl::optional_object<TLockedToken<CGenDescription>>& particle,
                      s16 w2, bool b2);
    void SetCameraShake(const CCameraShakeData& data) { x2fc_camShake = data; x3d0_27_camShakeDirty = true; }
};

}

#endif // __URDE_CENERGYPROJECTILE_HPP__
