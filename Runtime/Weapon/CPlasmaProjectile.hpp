#pragma once

#include "Weapon/CBeamProjectile.hpp"
#include "Weapon/CBeamInfo.hpp"
#include "World/CDamageInfo.hpp"

namespace urde
{
class CPlasmaProjectile : public CBeamProjectile
{
public:
    CPlasmaProjectile(const TToken<CWeaponDescription>& wDesc, std::string_view name,
                      EWeaponType wType, const CBeamInfo& bInfo, const zeus::CTransform& xf,
                      EMaterialTypes matType, const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid,
                      TUniqueId owner, u32 w1, bool b1, EProjectileAttrib attribs);

    void Accept(IVisitor& visitor);

    void UpdateFx(const zeus::CTransform&, float, CStateManager&) {}
    void Fire(const zeus::CTransform&, CStateManager&, bool);
};
}

