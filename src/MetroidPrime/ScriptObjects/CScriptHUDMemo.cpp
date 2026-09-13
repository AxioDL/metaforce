#include "MetroidPrime/ScriptObjects/CScriptHUDMemo.hpp"

#include "MetroidPrime/HUD/CSamusHud.hpp"

#include "Kyoto/Text/CStringTable.hpp"

CScriptHUDMemo::CScriptHUDMemo(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                               const CHUDMemoParms& parms, const EDisplayType disp, CAssetId msg,
                               const bool active)
: CEntity(uid, info, active, name)
, x34_parms(parms)
, x3c_dispType(disp)
, x40_stringTableId(msg)
, x44_stringTable(msg == kInvalidAssetId ? rstl::optional_object_null()
                                         : rstl::optional_object< TLockedToken< CStringTable > >(
                                               gpSimplePool->GetObj(SObjectTag('STRG', msg)))) {}

CScriptHUDMemo::~CScriptHUDMemo() {}

void CScriptHUDMemo::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_SetToZero:
    if (GetActive()) {
      if (x3c_dispType == kDT_MessageBox) {
        mgr.ShowPausedHUDMemo(x40_stringTableId, x34_parms.GetDisplayTime());
      } else {
        if (x44_stringTable) {
          CSamusHud::DisplayHudMemo((*x44_stringTable)->GetString(0), x34_parms);
        } else {
          CSamusHud::DisplayHudMemo(rstl::wstring_l(L""), x34_parms);
        }
      }
    }
    break;
  case kSM_Deactivate:
    if (GetActive() && x3c_dispType == kDT_StatusMessage) {
      CSamusHud::DisplayHudMemo(rstl::wstring_l(L""), CHUDMemoParms(0.f, false, true, false));
    }
    break;
  default:
    break;
  }

  CEntity::AcceptScriptMsg(msg, uid, mgr);
}

ENTITY_ACCEPT_IMPL(CScriptHUDMemo)
