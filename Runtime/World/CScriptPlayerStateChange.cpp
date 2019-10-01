#include "CScriptPlayerStateChange.hpp"
#include "CStateManager.hpp"
#include "CPlayerState.hpp"
#include "Input/ControlMapper.hpp"
#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
CScriptPlayerStateChange::CScriptPlayerStateChange(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                                   bool active, u32 itemType, u32 itemCount, u32 itemCapacity,
                                                   EControl control, EControlCommandOption controlCmdOpt)
: CEntity(uid, info, active, name)
, x34_itemType(itemType)
, x38_itemCount(itemCount)
, x3c_itemCapacity(itemCapacity)
, x40_ctrl(control)
, x44_ctrlCmdOpt(controlCmdOpt) {}

void CScriptPlayerStateChange::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptPlayerStateChange::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  if (GetActive() && msg == EScriptObjectMessage::SetToZero) {
    stateMgr.GetPlayerState()->AddPowerUp(CPlayerState::EItemType(x34_itemType), x3c_itemCapacity);
    stateMgr.GetPlayerState()->IncrPickup(CPlayerState::EItemType(x34_itemType), x38_itemCount);

    if (x44_ctrlCmdOpt == EControlCommandOption::Filtered && x40_ctrl == EControl::Filtered) {
      bool filtered = x44_ctrlCmdOpt != EControlCommandOption::Unfiltered;
      ControlMapper::SetCommandFiltered(ControlMapper::ECommands::OrbitClose, filtered);
      ControlMapper::SetCommandFiltered(ControlMapper::ECommands::OrbitConfirm, filtered);
      ControlMapper::SetCommandFiltered(ControlMapper::ECommands::OrbitDown, filtered);
      ControlMapper::SetCommandFiltered(ControlMapper::ECommands::OrbitFar, filtered);
      ControlMapper::SetCommandFiltered(ControlMapper::ECommands::OrbitLeft, filtered);
      ControlMapper::SetCommandFiltered(ControlMapper::ECommands::OrbitObject, filtered);
      ControlMapper::SetCommandFiltered(ControlMapper::ECommands::OrbitRight, filtered);
      ControlMapper::SetCommandFiltered(ControlMapper::ECommands::OrbitSelect, filtered);
      ControlMapper::SetCommandFiltered(ControlMapper::ECommands::OrbitUp, filtered);
    }
  }

  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
}
} // namespace urde