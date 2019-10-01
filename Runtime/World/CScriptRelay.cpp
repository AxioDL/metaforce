#include "CScriptRelay.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CScriptRelay::CScriptRelay(TUniqueId uid, std::string_view name, const CEntityInfo& info, bool active)
: CEntity(uid, info, active, name) {}

void CScriptRelay::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptRelay::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
  if (msg == EScriptObjectMessage::Deleted) {
    UpdateObjectRef(stateMgr);
  } else if (msg == EScriptObjectMessage::SetToZero) {
    if (!x30_24_active)
      return;

    x38_sendCount++;
    TUniqueId tmp = stateMgr.GetLastRelayId();
    while (tmp != GetUniqueId() && tmp != kInvalidUniqueId) {
      const CScriptRelay* obj = static_cast<const CScriptRelay*>(stateMgr.GetObjectById(tmp));
      if (!obj) {
        tmp = kInvalidUniqueId;
        break;
      }

      tmp = obj->x34_nextRelay;
    }

    if (tmp == kInvalidUniqueId) {
      x34_nextRelay = stateMgr.GetLastRelayId();
      stateMgr.SetLastRelayId(GetUniqueId());
    }
  }
}

void CScriptRelay::Think(float, CStateManager& stateMgr) {
  if (x38_sendCount == 0)
    return;

  while (x38_sendCount != 0) {
    x38_sendCount--;
    SendScriptMsgs(EScriptObjectState::Zero, stateMgr, EScriptObjectMessage::None);
  }
  UpdateObjectRef(stateMgr);
}

void CScriptRelay::UpdateObjectRef(CStateManager& stateMgr) {
  TUniqueId* tmp = stateMgr.GetLastRelayIdPtr();
  while (tmp != nullptr && *tmp != kInvalidUniqueId) {
    if (*tmp == GetUniqueId()) {
      *tmp = x34_nextRelay;
      return;
    }
    CScriptRelay* obj = static_cast<CScriptRelay*>(stateMgr.ObjectById(*tmp));
    if (obj == nullptr)
      return;
    tmp = &obj->x34_nextRelay;
  }
}

} // namespace urde
