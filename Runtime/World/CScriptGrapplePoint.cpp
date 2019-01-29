#include "CScriptGrapplePoint.hpp"
#include "Character/CModelData.hpp"
#include "Collision/CMaterialList.hpp"
#include "CActorParameters.hpp"
#include "TCastTo.hpp"

namespace urde {
CScriptGrapplePoint::CScriptGrapplePoint(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                         const zeus::CTransform& transform, bool active,
                                         const CGrappleParameters& params)
: CActor(uid, active, name, info, transform, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::Orbit),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_touchBounds(x34_transform.origin - 0.5f, x34_transform.origin + 0.5f)
, x100_parameters(params) {}

void CScriptGrapplePoint::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptGrapplePoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::Activate:
    if (!GetActive()) {
      AddMaterial(EMaterialTypes::Orbit, mgr);
      SetActive(true);
    }
    break;
  case EScriptObjectMessage::Deactivate:
    if (GetActive()) {
      RemoveMaterial(EMaterialTypes::Orbit, mgr);
      SetActive(false);
    }
    break;
  default:
    break;
  }
}

void CScriptGrapplePoint::Think(float, CStateManager&) {
  // Empty
}

void CScriptGrapplePoint::Render(const CStateManager&) const {
  // Empty
}

rstl::optional<zeus::CAABox> CScriptGrapplePoint::GetTouchBounds() const { return {xe8_touchBounds}; }

void CScriptGrapplePoint::AddToRenderer(const zeus::CFrustum&, const CStateManager& mgr) const {
  CActor::EnsureRendered(mgr);
}

} // namespace urde
