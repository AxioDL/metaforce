#include "Weapon/CBeamProjectile.hpp"
#include "TCastTo.hpp"

namespace urde
{

CBeamProjectile::CBeamProjectile(const TToken<CWeaponDescription>& wDesc, std::string_view name, EWeaponType wType,
                                 const zeus::CTransform& xf, s32 flags, float f1, float f2, EMaterialTypes matType,
                                 const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid, TUniqueId owner,
                                 EProjectileAttrib attribs, bool b1)
: CGameProjectile(false, wDesc, name, wType, xf, matType, dInfo, uid, aid, owner, kInvalidUniqueId, attribs, false,
                  zeus::CVector3f::skOne, {}, -1, false)
, x2e8_(std::abs(flags))
, x2ec_(x2e8_)
, x2f0_(1.f / x2ec_)
, x2f4_(f1)
, x300_(b1 == false ? x2ec_ : 0.f)
, x308_(f2)
, x464_24_(b1)
, x464_25_(false)
{

}

std::experimental::optional<zeus::CAABox> CBeamProjectile::GetTouchBounds() const
{
    if (!GetActive())
        return {};
    if (!x464_25_)
    {
        zeus::CVector3f pos = GetTranslation();
        return {{pos - 1.f, pos + 1.f}};
    }
    return {};
}

void CBeamProjectile::CalculateRenderBounds()
{
    x9c_renderBounds = x354_.getTransformedAABox(x324_);
}

void CBeamProjectile::ResetBeam(CStateManager &, bool)
{
    if (x464_24_)
        x300_ = 0.f;
}

void CBeamProjectile::UpdateFX(const zeus::CTransform &, float, CStateManager &)
{
}

void CBeamProjectile::Accept(urde::IVisitor& visitor)
{
    visitor.Visit(this);
}

}
