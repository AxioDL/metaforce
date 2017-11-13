#ifndef __URDE_CGAMEPROJECTILE_HPP__
#define __URDE_CGAMEPROJECTILE_HPP__

#include "Collision/CMaterialList.hpp"
#include "Weapon/CWeaponMode.hpp"
#include "Weapon/CWeapon.hpp"
#include "World/CDamageInfo.hpp"
#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "Weapon/CProjectileWeapon.hpp"

namespace urde
{
class CGenDescription;
class CWeaponDescription;
class CGameProjectile : public CWeapon
{
    //CProjectileWeapon x170_;
    TUniqueId x2c0_homingTargetId;
    TUniqueId x2c8_projectileLight;
    u32 x2cc_;
    union
    {
        struct
        {
            bool x2e4_24_ : 1;
            bool x2e4_25_ : 1;
            bool x2e4_26_waterUpdate : 1;
            bool x2e4_27_inWater : 1;
            bool x2e4_28_ : 1;
        };
    };
public:
    CGameProjectile(bool active, const TToken<CWeaponDescription>&, std::string_view name,
                    EWeaponType wType, const zeus::CTransform& xf, EMaterialTypes matType,
                    const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid, TUniqueId owner,
                    TUniqueId homingTarget, EProjectileAttrib attribs, bool underwater, const zeus::CVector3f& scale,
                    const rstl::optional_object<TLockedToken<CGenDescription>>& particle, s16 s1, bool b3);

    virtual void Accept(IVisitor &visitor);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager &);
    static EProjectileAttrib GetBeamAttribType(EWeaponType wType);
    void DeleteProjectileLight(CStateManager&);
    void CreateProjectileLight(std::string_view, const CLight&, CStateManager&);
    void Chase(float, CStateManager&);
    EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                           const CWeaponMode&, int) const
    { return EWeaponCollisionResponseTypes::Projectile; }
    TUniqueId GetHomingTargetId() const { return x2c0_homingTargetId; }
};
}

#endif // __URDE_CGAMEPROJECTILE_HPP__
