#ifndef __URDE_CBEAMPROJECTILE_HPP__
#define __URDE_CBEAMPROJECTILE_HPP__

#include "Weapon/CGameProjectile.hpp"
namespace urde
{
class CBeamProjectile : public CGameProjectile
{
    u32 x2e8_;
    float x2ec_;
    float x2f0_;
    float x2f4_;
    u32 x2f8_ = 0;
    TUniqueId x2fc_ = kInvalidUniqueId;
    TUniqueId x2fe_ = kInvalidUniqueId;
    float x300_;
    float x304_;
    float x308_;
    zeus::CVector3f x30c_ = zeus::CVector3f::skUp;
    zeus::CTransform x324_;
    zeus::CAABox x354_ = zeus::CAABox::skNullBox;
    rstl::reserved_vector<zeus::CVector3f, 10> x384_;
    rstl::reserved_vector<zeus::CVector3f, 8> x400_;
    bool x464_24_ : 1;
    bool x464_25_ : 1;
public:
    CBeamProjectile(const TToken<CWeaponDescription>& wDesc, std::string_view name, EWeaponType wType,
                    const zeus::CTransform& xf, s32 flags, float f1, float f2, EMaterialTypes matType,
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

    std::experimental::optional<zeus::CAABox> GetTouchBounds() const;
    void CalculateRenderBounds();
    virtual void ResetBeam(CStateManager&, bool);
    virtual void UpdateFX(const zeus::CTransform&, float, CStateManager&);
};
}
#endif // __URDE_CBEAMPROJECTILE_HPP__
