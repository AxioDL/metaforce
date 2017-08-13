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
    TUniqueId x2c8_projectileLight;
    u32 x2cc_;
    union
    {
        struct
        {
            bool x2e4_24_ : 1;
            bool x2e4_25_ : 1;
            bool x2e4_26_ : 1;
            bool x2e4_27_ : 1;
            bool x2e4_28_ : 1;
        };
    };
public:
    CGameProjectile(bool, const TToken<CWeaponDescription>&, const std::string&, EWeaponType, const zeus::CTransform&,
                    EMaterialTypes, const CDamageInfo&, TUniqueId, TAreaId, TUniqueId, TUniqueId, u32, bool,
                    const zeus::CVector3f&, const rstl::optional_object<TLockedToken<CGenDescription>>&, s16, bool);

    virtual void Accept(IVisitor &visitor);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager &);
    static EProjectileAttrib GetBeamAttribType(EWeaponType wType);
    void DeleteProjectileLight(CStateManager&);
    void CreateProjectileLight(const std::string&, const CLight&, CStateManager&);
    void Chase(float, CStateManager&);
    EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                           const CWeaponMode&, int) const
    { return EWeaponCollisionResponseTypes::Projectile; }
};
}

#endif // __URDE_CGAMEPROJECTILE_HPP__
