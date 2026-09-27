#include "MetroidPrime/ScriptObjects/CScriptPointOfInterest.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

#include "Kyoto/Math/CAABox.hpp"

CScriptPointOfInterest::CScriptPointOfInterest(TUniqueId uid, const rstl::string& name,
                                               const CEntityInfo& info, const CTransform4f& xf,
                                               const bool active, const CScannableParameters& parms,
                                               float f1)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_Orbit),
         CActorParameters::None().Scannable(parms), kInvalidUniqueId)
, mPointSize(f1) {}

CScriptPointOfInterest::~CScriptPointOfInterest() {}

void CScriptPointOfInterest::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                             CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);
}

rstl::optional_object< CAABox > CScriptPointOfInterest::GetTouchBounds() const {
  return CAABox(GetTranslation(), GetTranslation());
}

void CScriptPointOfInterest::Render(const CStateManager&) const {}

void CScriptPointOfInterest::AddToRenderer(const CFrustumPlanes&, const CStateManager&) const {}

ENTITY_ACCEPT_IMPL(CScriptPointOfInterest)

void CScriptPointOfInterest::Think(float dt, CStateManager& mgr) {
  SetTargetable(mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan);
  CEntity::Think(dt, mgr);
}

void CScriptPointOfInterest::CalculateRenderBounds() {
  if (mPointSize == 0.f) {
    CActor::CalculateRenderBounds();
  } else {
    CVector3f origin = GetTranslation();
    SetRenderBounds(CAABox(origin - CVector3f(mPointSize, mPointSize, mPointSize),
                           origin + CVector3f(mPointSize, mPointSize, mPointSize)));
  }
}
