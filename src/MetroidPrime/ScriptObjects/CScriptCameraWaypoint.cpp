#include "MetroidPrime/ScriptObjects/CScriptCameraWaypoint.hpp"

#include "MetroidPrime/CActorParameters.hpp"

CScriptCameraWaypoint::CScriptCameraWaypoint(TUniqueId uid, const rstl::string& name,
                                             const CEntityInfo& info, const CTransform4f& xf,
                                             const bool active, float hfov, uint w1)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_hfov(hfov)
, xec_(w1) {}

CScriptCameraWaypoint::~CScriptCameraWaypoint() {}

void CScriptCameraWaypoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                            CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);
  if (GetActive()) {
    switch (msg) {
    case kSM_Arrived:
      SendScriptMsgs(kSS_Arrived, mgr, kSM_None);
      break;
    default:
      break;
    }
  }
}

TUniqueId CScriptCameraWaypoint::NextWaypoint(CStateManager& mgr) {
  rstl::vector< TUniqueId > candidateIds;

  rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
  for (; conn != GetConnectionList().end(); ++conn) {
    if (conn->x0_state == kSS_Arrived && conn->x4_msg == kSM_Next) {
      TUniqueId uid = mgr.GetIdForScript(conn->x8_objId);
      if (uid == kInvalidUniqueId) {
        continue;
      }
      candidateIds.reserve(candidateIds.size() + 1);
      candidateIds.push_back(uid);
    }
  }

  if (candidateIds.empty())
    return kInvalidUniqueId;

  return candidateIds[mgr.Random()->Range(0, s32(candidateIds.size() - 1))];
}

void CScriptCameraWaypoint::AddToRenderer(const CFrustumPlanes&, const CStateManager&) const {}

void CScriptCameraWaypoint::Render(const CStateManager&) const {}

ENTITY_ACCEPT_IMPL(CScriptCameraWaypoint)
