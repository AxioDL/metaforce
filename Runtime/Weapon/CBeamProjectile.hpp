#ifndef __URDE_CBEAMPROJECTILE_HPP__
#define __URDE_CBEAMPROJECTILE_HPP__

#include "Weapon/CGameProjectile.hpp"
namespace urde
{
class CBeamProjectile : public CGameProjectile
{
public:
    CBeamProjectile(const TToken<CWeaponDescription>& wDesc, const std::string& name, EWeaponType wType,
                    const zeus::CTransform& xf, int flags, float f1, float f2, EMaterialTypes matType,
                    const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid, TUniqueId owner,
                    EProjectileAttrib attribs, bool b1);

    void Accept(IVisitor &visitor);
    float GetMaxRadius() const;
    zeus::CVector3f GetSurfaceNormal() const;
    void GetDamageType() const;
    void GetCurrentPos() const;
    void PointCache();
    void GetPointCache() const;
    void CauseDamage(bool);
    zeus::CVector3f GetBeamOrigin() const;
    void GetInvMaxLength() const;
    void GetCurrentLength();
    void GetMaxLength();
    s32 GetIntMaxLength();

    void Think(float, CStateManager&);
    rstl::optional_object<zeus::CAABox> GetTouchBounds() const;
    void Touch(CActor&, CStateManager&);
    virtual void ResetBeam(CStateManager&, bool);
    virtual void UpdateFX(const zeus::CTransform&, float, CStateManager&);
};
}
#endif // __URDE_CBEAMPROJECTILE_HPP__
