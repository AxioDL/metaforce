#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CStateManager.hpp"

CScriptWaypoint::CScriptWaypoint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                                 const CTransform4f& xf, const bool active, float speed,
                                 float pause, int patternTranslate, int patternOrient,
                                 int patternFit, int behaviour, int behaviourOrient,
                                 int behaviourModifiers, uint animation)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_speed(speed)
, xec_animation(animation)
, xf0_pause(pause)
, xf4_patternTranslate(patternTranslate)
, xf5_patternOrient(patternOrient)
, xf6_patternFit(patternFit)
, xf7_behaviour(behaviour)
, xf8_behaviourOrient(behaviourOrient)
, xfa_behaviourModifiers(behaviourModifiers) {
  SetUseInSortedLists(false);
  SetCallTouch(false);
}

CScriptWaypoint::~CScriptWaypoint() {}

void CScriptWaypoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender,
                                      CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, sender, mgr);
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

TUniqueId CScriptWaypoint::NextWaypoint(const CStateManager& mgr) const {
  rstl::reserved_vector< TUniqueId, 10 > ids;
  rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
  for (; conn != GetConnectionList().end(); ++conn) {
    if (conn->x0_state == kSS_Arrived && conn->x4_msg == kSM_Next) {
      const TUniqueId id = mgr.GetIdForScript(conn->x8_objId);
      if (id != kInvalidUniqueId) {
        if (const CScriptWaypoint* wp = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id))) {
          if (wp->GetActive()) {
            ids.push_back(id);
          }
        }
      }
    }
  }

  if (ids.empty()) {
    return kInvalidUniqueId;
  }

  return ids[int(mgr.Random()->Float() * ids.size() * 0.99f)];
}

TUniqueId CScriptWaypoint::FollowWaypoint(CStateManager& mgr) const {
  rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
  for (; conn != GetConnectionList().end(); ++conn) {
    if (conn->x0_state == kSS_Arrived && conn->x4_msg == kSM_Follow) {
      return mgr.GetIdForScript(conn->x8_objId);
    }
  }
  return kInvalidUniqueId;
}

void CScriptWaypoint::AddToRenderer(const CFrustumPlanes&, const CStateManager&) const {
  // Empty
}

void CScriptWaypoint::Render(const CStateManager&) const {}

ENTITY_ACCEPT_IMPL(CScriptWaypoint)
