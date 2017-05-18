#ifndef __URDE_CFLAMETHROWER_HPP__
#define __URDE_CFLAMETHROWER_HPP__

#include "Weapon/CGameProjectile.hpp"

namespace urde
{
class CFlameInfo;
class CElementGen;
class CFlameThrower : public CGameProjectile
{
    static const zeus::CVector3f kLightOffset;
    zeus::CTransform x2e8_;
    zeus::CAABox x318_ = zeus::CAABox::skNullBox;
    TToken<CGenDescription> x33c_flameDesc;
    std::unique_ptr<CElementGen> x348_flameGen;

    union
    {
        struct
        {
            bool x400_25 : 1;
        };
        u32 _dummy = 0;
    };
public:
    CFlameThrower(const TToken<CWeaponDescription>& wDesc, const std::string& name, EWeaponType wType,
                  const CFlameInfo& flameInfo, const zeus::CTransform& xf, EMaterialTypes matType,
                  const CDamageInfo& dInfo, TUniqueId owner, TAreaId aId, TUniqueId uid, u32 w1);

    void Accept(IVisitor &visitor);
    void SetTransform(const zeus::CTransform& xf);
    void Reset(CStateManager&, bool);
    void Fire(const zeus::CTransform&, CStateManager&, bool);
    bool GetX400_25() const { return x400_25; }
};
}
#endif // __URDE_CFLAMETHROWER_HPP__
