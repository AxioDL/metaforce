#include "MetroidPrime/ScriptObjects/CScriptRelay.hpp"

CScriptRelay::CScriptRelay(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const bool active)
: CEntity(uid, info, active, name), mNextRelay(kInvalidUniqueId), mSendCount(0) {}

void CScriptRelay::UpdateObjectRef(CStateManager& stateMgr) {
  TUniqueId* tmp = stateMgr.GetLastRelayIdPtr();
  while (tmp != nullptr && *tmp != kInvalidUniqueId) {
    if (*tmp == GetUniqueId()) {
      *tmp = mNextRelay;
      return;
    }
    CScriptRelay* obj = static_cast< CScriptRelay* >(stateMgr.ObjectById(*tmp));
    if (obj == nullptr) {
      return;
    }
    tmp = &obj->mNextRelay;
  }
}

void CScriptRelay::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId,
                                   CStateManager& stateMgr) {
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);

  switch (msg) {
  case kSM_SetToZero: {
    if (!GetActive()) {
      return;
    }

    mSendCount++;
    TUniqueId tmp = stateMgr.GetLastRelayId();
    while (tmp != GetUniqueId() && tmp != kInvalidUniqueId) {
      CEntity* obj = stateMgr.ObjectById(tmp);
      if (!obj) {
        tmp = kInvalidUniqueId;
        break;
      }

      tmp = static_cast< const CScriptRelay* >(obj)->mNextRelay;
    }

    if (tmp == kInvalidUniqueId) {
      mNextRelay = stateMgr.GetLastRelayId();
      stateMgr.SetLastRelayId(GetUniqueId());
    }
    break;
  }
  case kSM_Deleted:
    UpdateObjectRef(stateMgr);
    break;
  }
}

void CScriptRelay::Think(float, CStateManager& stateMgr) {
  switch (mSendCount) {
  default:
    while (mSendCount != 0) {
      mSendCount--;
      SendScriptMsgs(kSS_Zero, stateMgr, kSM_None);
    }
    UpdateObjectRef(stateMgr);
  case 0:
    return;
  }
}

ENTITY_ACCEPT_IMPL(CScriptRelay)
