#include "MetroidPrime/ScriptObjects/CScriptPlayerStateChange.hpp"

#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

CScriptPlayerStateChange::CScriptPlayerStateChange(TUniqueId uid, const rstl::string& name,
                                                   const CEntityInfo& info, const bool active,
                                                   const int itemType, const int itemCount,
                                                   const int itemCapacity, const EControl control,
                                                   const EControlCommandOption controlCmdOpt)
: CEntity(uid, info, active, name)
, x34_itemType(itemType)
, x38_itemCount(itemCount)
, x3c_itemCapacity(itemCapacity)
, x40_ctrl(control)
, x44_ctrlCmdOpt(controlCmdOpt) {}

ENTITY_ACCEPT_IMPL(CScriptPlayerStateChange)

void CScriptPlayerStateChange::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId,
                                               CStateManager& stateMgr) {
  if (GetActive() && msg == kSM_SetToZero) {
    stateMgr.PlayerState()->InitializePowerUp(CPlayerState::EItemType(x34_itemType),
                                              x3c_itemCapacity);
    stateMgr.PlayerState()->IncrPickUp(CPlayerState::EItemType(x34_itemType), x38_itemCount);

    if (x44_ctrlCmdOpt != kCCO_Unfiltered) {
      switch (x40_ctrl) {
      case kC_Filtered:
        bool filtered = Filtered();
        ControlMapper::SetCommandFiltered(ControlMapper::kC_OrbitClose, filtered);
        ControlMapper::SetCommandFiltered(ControlMapper::kC_OrbitConfirm, filtered);
        ControlMapper::SetCommandFiltered(ControlMapper::kC_OrbitDown, filtered);
        ControlMapper::SetCommandFiltered(ControlMapper::kC_OrbitFar, filtered);
        ControlMapper::SetCommandFiltered(ControlMapper::kC_OrbitLeft, filtered);
        ControlMapper::SetCommandFiltered(ControlMapper::kC_OrbitObject, filtered);
        ControlMapper::SetCommandFiltered(ControlMapper::kC_OrbitRight, filtered);
        ControlMapper::SetCommandFiltered(ControlMapper::kC_OrbitSelect, filtered);
        ControlMapper::SetCommandFiltered(ControlMapper::kC_OrbitUp, filtered);
      }
    }
  }

  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
}

CScriptPlayerStateChange::~CScriptPlayerStateChange() {}
