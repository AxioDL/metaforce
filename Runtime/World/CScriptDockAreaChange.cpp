#include "Runtime/World/CScriptDockAreaChange.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CScriptDock.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
CScriptDockAreaChange::CScriptDockAreaChange(TUniqueId uid, std::string_view name, const CEntityInfo& info, s32 w1,
                                             bool active)
: CEntity(uid, info, active, name), x34_dockReference(w1) {}

void CScriptDockAreaChange::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptDockAreaChange::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  if (msg == EScriptObjectMessage::Action && GetActive()) {
    for (SConnection conn : x20_conns) {
      if (conn.x0_state != EScriptObjectState::Play)
        continue;

      auto search = stateMgr.GetIdListForScript(conn.x8_objId);
      for (auto it = search.first; it != search.second; ++it) {
        TUniqueId id = it->second;
        TCastToPtr<CScriptDock> dock(stateMgr.ObjectById(id));
        if (dock)
          dock->SetDockReference(stateMgr, x34_dockReference);
      }
    }

    SendScriptMsgs(EScriptObjectState::Play, stateMgr, EScriptObjectMessage::None);
  }

  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
}
} // namespace urde
