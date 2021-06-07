#include "Runtime/MP1/World/CIceAttackProjectile.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CActorParameters.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce::MP1 {

CIceAttackProjectile::CIceAttackProjectile(const TToken<CGenDescription>& gen1, const TToken<CGenDescription>& gen2,
                                           const TToken<CGenDescription>& gen3, TUniqueId uid, TAreaId areaId,
                                           TUniqueId owner, bool active, const zeus::CTransform& xf,
                                           const CDamageInfo& dInfo, const zeus::CAABox& bounds, float f1, float f2,
                                           CAssetId unkInt1, u16 unkShort1, u16 unkShort2, CAssetId unkInt2)
: CActor(uid, active, "IceAttackProjectile"sv, CEntityInfo(areaId, NullConnectionList), xf,
         CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::Projectile, EMaterialTypes::CameraPassthrough),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_(gen1)
, xf0_(gen2)
, xf8_(gen3)
, x118_owner(owner)
, x11c_(dInfo)
, x138_(dInfo)
, x154_(bounds)
, x170_(f1)
, x174_(f2)
, x184_(unkInt1)
, x188_(unkShort1)
, x18a_(unkShort2)
, x18c_(unkInt2) {

  zeus::CVector3f m0 = zeus::CVector3f{0.f, 1.f, 0.f}.cross(xf.frontVector()).normalized();
  zeus::CVector3f m1 = m0.cross(zeus::CVector3f{0.f, 0.f, 1.f}).normalized();
  SetTransform(zeus::CTransform(m0, m1, zeus::CVector3f{0.f, 0.f, 1.f}, GetTranslation()));
  x100_ = std::make_unique<CElementGen>(xf8_, CElementGen::EModelOrientationType::Normal,
                                        CElementGen::EOptionalSystemFlags::One);
}

void CIceAttackProjectile::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CIceAttackProjectile::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (!x190_) {}
  CEntity::Think(dt, mgr);
}

void CIceAttackProjectile::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, other, mgr);
}

void CIceAttackProjectile::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  mgr.AddDrawableActor(*this, -1000.f * CGraphics::g_ViewMatrix.frontVector(), x9c_renderBounds);
}

void CIceAttackProjectile::Render(CStateManager& mgr) {}

void CIceAttackProjectile::Touch(CActor& act, CStateManager& mgr) {}
} // namespace metaforce::MP1