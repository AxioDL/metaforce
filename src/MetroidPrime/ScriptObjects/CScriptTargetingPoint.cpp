#include "MetroidPrime/ScriptObjects/CScriptTargetingPoint.hpp"

#include "MetroidPrime/CActorParameters.hpp"

CScriptTargetingPoint::CScriptTargetingPoint(TUniqueId uid, const rstl::string& name,
                                             const CEntityInfo& info, const CTransform4f& xf,
                                             const bool active)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, mE4_(false)
, xea_(kInvalidUniqueId)
, mTime(0.f) {}

ENTITY_ACCEPT_IMPL(CScriptTargetingPoint)

void CScriptTargetingPoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                            CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case kSM_Deactivate:
  case kSM_Activate:
    CEntity::SendScriptMsgs(kSS_Attack, mgr, kSM_None);
  }
}

bool CScriptTargetingPoint::GetLocked() const { return GetConnectionList().size() > 0; }

void CScriptTargetingPoint::Think(float dt, CStateManager&) {
  if (mTime > 0.f) {
    mTime -= dt;
  }
}

void CScriptTargetingPoint::AddToRenderer(const CFrustumPlanes&, const CStateManager&) const {}

void CScriptTargetingPoint::Render(const CStateManager&) const {}

CScriptTargetingPoint::~CScriptTargetingPoint() {}
