#include "MetroidPrime/ScriptObjects/CScriptGrapplePoint.hpp"

#include "MetroidPrime/CActorParameters.hpp"

CScriptGrapplePoint::CScriptGrapplePoint(TUniqueId uid, const rstl::string& name,
                                         const CEntityInfo& info, const CTransform4f& xf,
                                         const bool active, const CGrappleParameters& params)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_Orbit),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_touchBounds(GetTranslation() - CVector3f(0.5f, 0.5f, 0.5f),
                  GetTranslation() + CVector3f(0.5f, 0.5f, 0.5f))
, x100_parameters(params) {}

CScriptGrapplePoint::~CScriptGrapplePoint() {}

void CScriptGrapplePoint::AddToRenderer(const CFrustumPlanes&, const CStateManager& mgr) const {
  CActor::EnsureRendered(mgr);
}

rstl::optional_object< CAABox > CScriptGrapplePoint::GetTouchBounds() const {
  return xe8_touchBounds;
}

void CScriptGrapplePoint::Render(const CStateManager&) const {}

void CScriptGrapplePoint::Think(float, CStateManager&) {}

void CScriptGrapplePoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                          CStateManager& mgr) {
  switch (msg) {
  case kSM_Activate:
    if (!GetActive()) {
      AddMaterial(kMT_Orbit, mgr);
      SetActive(true);
    }
    break;
  case kSM_Deactivate:
    if (GetActive()) {
      RemoveMaterial(kMT_Orbit, mgr);
      SetActive(false);
    }
    break;
  default:
    break;
  }
}

ENTITY_ACCEPT_IMPL(CScriptGrapplePoint)
