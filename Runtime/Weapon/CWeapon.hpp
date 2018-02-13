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
protected:
    EProjectileAttrib xe8_projectileAttribs;
    TUniqueId xec_ownerId;
    EWeaponType xf0_weaponType;
    CMaterialFilter xf8_filter;
    CDamageInfo x110_origDamageInfo;
    CDamageInfo x12c_curDamageInfo;
    float x148_curTime = 0.f;
    float x14c_damageFalloffSpeed = 0.f;
    float x150_damageDuration = 0.f;
    float x154_interferenceDuration = 0.f;
public:
    CWeapon(TUniqueId uid, TAreaId aid, bool active, TUniqueId owner, EWeaponType type,
            std::string_view name, const zeus::CTransform& xf, const CMaterialFilter& filter,
            const CMaterialList& mList, const CDamageInfo&, EProjectileAttrib attribs, CModelData&& mData);

    virtual void Accept(IVisitor &visitor);
    bool HasAttrib(EProjectileAttrib attrib) const
    { return (int(xe8_projectileAttribs) & int(attrib)) == int(attrib); }
    EProjectileAttrib GetAttribField() const { return xe8_projectileAttribs; }
    const CMaterialFilter& GetFilter() const { return xf8_filter; }
    void SetFilter(const CMaterialFilter& filter) { xf8_filter = filter; }
    TUniqueId GetOwnerId() const { return xec_ownerId; }
    void SetOwnerId(TUniqueId oid) { xec_ownerId = oid; }
    EWeaponType GetType() const { return xf0_weaponType; }
    const CDamageInfo& GetDamageInfo() const { return x12c_curDamageInfo; }
    CDamageInfo& DamageInfo() { return x12c_curDamageInfo; }
    void SetDamageInfo(const CDamageInfo& dInfo) { x12c_curDamageInfo = dInfo; }
    float GetDamageDuration() const { return x150_damageDuration; }
    float GetInterferenceDuration() const { return x154_interferenceDuration; }

    void Think(float, CStateManager &);
    void Render(const CStateManager&) const;
    EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                           const CWeaponMode&, EProjectileAttrib) const;
    void FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr);
};
}
#endif // __URDE_CWEAPON_HPP__
