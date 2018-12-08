#include "CScriptSpiderBallAttractionSurface.hpp"
#include "CActorParameters.hpp"
#include "TCastTo.hpp"

namespace urde {

CScriptSpiderBallAttractionSurface::CScriptSpiderBallAttractionSurface(TUniqueId uid, std::string_view name,
                                                                       const CEntityInfo& info,
                                                                       const zeus::CTransform& xf,
                                                                       const zeus::CVector3f& scale, bool active)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), {EMaterialTypes::NoStepLogic},
         CActorParameters::None(), kInvalidUniqueId)
, xe8_scale(scale)
, xf4_aabb(zeus::CAABox(scale * -0.5f, scale * 0.5f).getTransformedAABox(xf.getRotation())) {}

void CScriptSpiderBallAttractionSurface::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptSpiderBallAttractionSurface::Think(float dt, CStateManager& mgr) {
  // Empty
}

void CScriptSpiderBallAttractionSurface::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender,
                                                         CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, sender, mgr);
}

std::experimental::optional<zeus::CAABox> CScriptSpiderBallAttractionSurface::GetTouchBounds() const {
  if (GetActive())
    return {zeus::CAABox(xf4_aabb.min + GetTranslation(), xf4_aabb.max + GetTranslation())};
  return {};
}

void CScriptSpiderBallAttractionSurface::Touch(CActor& actor, CStateManager& mgr) {
  // Empty
}

} // namespace urde
