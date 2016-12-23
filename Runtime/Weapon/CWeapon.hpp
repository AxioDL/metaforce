#ifndef __URDE_CWEAPON_HPP__
#define __URDE_CWEAPON_HPP__

#include "World/CActor.hpp"
#include "Weapon/WeaponCommon.hpp"
#include "Collision/CMaterialFilter.hpp"

namespace urde
{
class CWeapon : public CActor
{
public:
    enum class EProjectileAttrib
    {
        None = 0,
        Ice = (1 << 3),
        Wave = (1 << 4),
        Plasma = (1 << 5),
        Phazon = (1 << 6)
    };

private:
public:
    CWeapon(TUniqueId, TAreaId, bool, TUniqueId, EWeaponType, const std::string&, const zeus::CTransform&,
            const CMaterialFilter&, const CMaterialList&, const CDamageInfo&, EProjectileAttrib, CModelData&&);

    bool HasAttrib(EProjectileAttrib) const;
    EProjectileAttrib GetAttribField() const;
    const CMaterialFilter& GetFilter() const;
    void SetFilter(const CMaterialFilter&);
    TUniqueId GetOwnerId() const;
    void SetOwnerId(TUniqueId);
    EWeaponType GetType() const;
    const CDamageInfo& GetDamageInfo() const;
    CDamageInfo& DamageInfo();
    void SetDamageInfo(const CDamageInfo&);

    void Think(float, CStateManager &) {}
    void Render(const CStateManager&) const {}
    EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&, CWeaponMode&,
                                                           int)
    {
        return EWeaponCollisionResponseTypes::Default;
    }
    void FluidFXThink(EFluidState, CScriptWater&, CStateManager&) {}
};
ENABLE_BITWISE_ENUM(CWeapon::EProjectileAttrib)
}
#endif // __URDE_CWEAPON_HPP__
