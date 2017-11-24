#ifndef __URDE_CWEAPON_HPP__
#define __URDE_CWEAPON_HPP__

#include "World/CActor.hpp"
#include "Weapon/WeaponCommon.hpp"
#include "World/CDamageInfo.hpp"
#include "Collision/CMaterialFilter.hpp"

namespace urde
{
class CWeapon : public CActor
{
public:
    enum class EProjectileAttrib
    {
        None = 0,
        PartialCharge = (1 << 0),
        PlasmaProjectile = (1 << 1),
        Charged = (1 << 2),
        Ice = (1 << 3),
        Wave = (1 << 4),
        Plasma = (1 << 5),
        Phazon = (1 << 6),
        Unknown1 = (1 << 7),
        Bombs = (1 << 8),
        PowerBombs = (1 << 9),
        ArmCannon = (1 << 11),
        BigStrike = (1 << 12),
        StaticInterference = (1 << 14),
        KeepInCinematic = (1 << 17),
    };

private:
    EProjectileAttrib xe8_projectileAttribs;
    TUniqueId xec_ownerId;
    EWeaponType xf0_weaponType;
    u32 xf4_;
    CMaterialFilter xf8_;
    u32 x10c_;
    CDamageInfo x110_;
    CDamageInfo x12c_;
    float x148_;
    float x14c_;
    float x150_damageDuration;
    float x154_interferenceDuration;
public:
    CWeapon(TUniqueId uid, TAreaId aid, bool active, TUniqueId owner, EWeaponType type,
            std::string_view name, const zeus::CTransform& xf, const CMaterialFilter& filter,
            const CMaterialList& mList, const CDamageInfo&, EProjectileAttrib attribs, CModelData&& mData);

    virtual void Accept(IVisitor &visitor);
    bool HasAttrib(EProjectileAttrib) const;
    EProjectileAttrib GetAttribField() const { return xe8_projectileAttribs; }
    const CMaterialFilter& GetFilter() const;
    void SetFilter(const CMaterialFilter&);
    TUniqueId GetOwnerId() const { return xec_ownerId; }
    void SetOwnerId(TUniqueId oid) { xec_ownerId = oid; }
    EWeaponType GetType() const;
    const CDamageInfo& GetDamageInfo() const;
    CDamageInfo& DamageInfo();
    void SetDamageInfo(const CDamageInfo&);
    float GetDamageDuration() const { return x150_damageDuration; }
    float GetInterferenceDuration() const { return x154_interferenceDuration; }

    void Think(float, CStateManager &) {}
    void Render(const CStateManager&) const {}
    EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                           const CWeaponMode&, int) const
    {
        return EWeaponCollisionResponseTypes::Default;
    }
    void FluidFXThink(EFluidState, CScriptWater&, CStateManager&) {}
};
ENABLE_BITWISE_ENUM(CWeapon::EProjectileAttrib)
}
#endif // __URDE_CWEAPON_HPP__
