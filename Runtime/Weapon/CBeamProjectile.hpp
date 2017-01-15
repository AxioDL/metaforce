#ifndef __URDE_CBEAMPROJECTILE_HPP__
#define __URDE_CBEAMPROJECTILE_HPP__

#include "Weapon/CGameProjectile.hpp"
namespace urde
{
class CBeamProjectile : public CGameProjectile
{
public:
    CBeamProjectile(const TToken<CWeaponDescription>&, const std::string&, EWeaponType, const zeus::CTransform&, int,
                    float, float, EMaterialTypes, const CDamageInfo&, TUniqueId, TAreaId, TUniqueId, u32, bool);

    virtual void Accept(IVisitor &visitor);
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
