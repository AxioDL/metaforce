#ifndef __URDE_CPLASMAPROJECTILE_HPP__
#define __URDE_CPLASMAPROJECTILE_HPP__

#include "Weapon/CBeamProjectile.hpp"
#include "Weapon/CBeamInfo.hpp"
#include "World/CDamageInfo.hpp"

namespace urde
{
class CPlasmaProjectile : public CBeamProjectile
{
public:
    CPlasmaProjectile(const TToken<CWeaponDescription>&, const std::string&, EWeaponType, const CBeamInfo&,
                      const zeus::CTransform&, EMaterialTypes, const CDamageInfo&, TUniqueId, TAreaId, TUniqueId, u32,
                      bool, u32);

    void Accept(IVisitor& visitor);
};
}

#endif // __URDE_CPLASMAPROJECTILE_HPP__
