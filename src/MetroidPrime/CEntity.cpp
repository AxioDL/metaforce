#include "MetroidPrime/CEntity.hpp"

#include "MetroidPrime/CValidEntityPredicate.hpp"

rstl::vector< SConnection > CEntity::NullConnectionList;

CEntityInfo::CEntityInfo(const TAreaId aid, const rstl::vector< SConnection >& conns, TEditorId eid)
: mAreaId(aid), mConns(conns), mEditorId(eid) {}

u64 CEntityInfo::GetGloballyUniqueIdForScriptObject(uint worldId, TEditorId editorId) {
  return (static_cast< u64 >(worldId) << 32) | editorId.value;
}

u64 CEntityInfo::GetGloballyUniqueIdForScriptObject(uint worldId) const {
  return (static_cast< u64 >(worldId) << 32) | mEditorId.value;
}

CEntity::CEntity(const TUniqueId id, const CEntityInfo& info, bool active, const rstl::string& name)
: mAreaId(info.GetAreaId())
, mUid(id)
, mEditorId(info.GetEditorId())
, mName(name)
, mConns(info.GetConnectionList())
, mActive(active)
, mInGraveyard(false)
, mScriptingBlocked(false)
, mNotInArea(mAreaId == kInvalidAreaId) {}

CEntity::~CEntity() {}

void CEntity::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_Activate:
    if (!mActive) {
      SetActive(true);
      SendScriptMsgs(kSS_Active, mgr, kSM_None);
    }
    break;
  case kSM_Deactivate:
    if (mActive) {
      SetActive(false);
      SendScriptMsgs(kSS_Inactive, mgr, kSM_None);
    }
    break;
  case kSM_ToggleActive:
    if (!mActive) {
      AcceptScriptMsg(kSM_Activate, uid, mgr);
    } else {
      AcceptScriptMsg(kSM_Deactivate, uid, mgr);
    }
    break;
  }
}

void CEntity::SendScriptMsgs(EScriptObjectState state, CStateManager& mgr,
                             EScriptObjectMessage skipMsg) {
  rstl::vector< SConnection >::const_iterator it = mConns.begin();
  for (; it != mConns.end(); ++it) {
    if (it->mState == state && it->mMsg != skipMsg) {
      mgr.SendScriptMsg(GetUniqueId(), it->mObjId, it->mMsg, state);
    }
  }
}

void CEntity::PreThink(float dt, CStateManager& mgr) {}

void CEntity::Think(float dt, CStateManager& mgr) {}

void CEntity::SetActive(const bool active) { mActive = active; }

const TAreaId CEntity::GetAreaId() const { return mNotInArea ? kInvalidAreaId : mAreaId; }

TUniqueId CEntity::CheckConnectedObject_if(const CStateManager& mgr, EScriptObjectState state,
                                          EScriptObjectMessage msg,
                                          const CValidEntityPredicate& predicate) const {
  for (AUTO(conn, mConns.begin()); conn != mConns.end(); conn++) {
    if ((state == kSS_Any || state == conn->mState) &&
        (msg == kSM_None || msg == conn->mMsg)) {
      CStateManager::TIdListResult ids = mgr.GetIdListForScript(conn->mObjId);
      for (AUTO(it, ids.first); it != ids.second; ++it) {
        if (predicate(mgr, it->second)) {
          return it->second;
        }
      }
    }
  }
  return kInvalidUniqueId;
}

#if VERSION >= VERSION_R3IJ_00
// This unit provides the shared Trilogy connection-vector instantiation.
template void rstl::vector< SConnection >::reserve(int);
#endif
