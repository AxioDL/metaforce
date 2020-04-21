#include "Runtime/World/CEntity.hpp"

#include "Runtime/CStateManager.hpp"

namespace urde {
const std::vector<SConnection> CEntity::NullConnectionList;

CEntity::CEntity(TUniqueId uniqueId, const CEntityInfo& info, bool active, std::string_view name)
: x4_areaId(info.GetAreaId())
, x8_uid(uniqueId)
, xc_editorId(info.GetEditorId())
, x10_name(name)
, x20_conns(info.GetConnectionList())
, x30_24_active(active)
, x30_27_inUse(x4_areaId != kInvalidAreaId) {}

void CEntity::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  switch (msg) {
  case EScriptObjectMessage::Activate:
    if (!GetActive()) {
      SetActive(true);
      SendScriptMsgs(EScriptObjectState::Active, stateMgr, EScriptObjectMessage::None);
    }
    break;
  case EScriptObjectMessage::Deactivate:
    if (GetActive()) {
      SetActive(false);
      SendScriptMsgs(EScriptObjectState::Inactive, stateMgr, EScriptObjectMessage::None);
    }
    break;
  case EScriptObjectMessage::ToggleActive:
    if (GetActive()) {
      SetActive(false);
      SendScriptMsgs(EScriptObjectState::Inactive, stateMgr, EScriptObjectMessage::None);
    } else {
      SetActive(true);
      SendScriptMsgs(EScriptObjectState::Active, stateMgr, EScriptObjectMessage::None);
    }
    break;
  default:
    break;
  }
}

void CEntity::SendScriptMsgs(EScriptObjectState state, CStateManager& stateMgr, EScriptObjectMessage skipMsg) {
  for (const SConnection& conn : x20_conns) {
    if (conn.x0_state == state && conn.x4_msg != skipMsg) {
      stateMgr.SendScriptMsg(x8_uid, conn.x8_objId, conn.x4_msg, state);
    }
  }
}
} // namespace urde
