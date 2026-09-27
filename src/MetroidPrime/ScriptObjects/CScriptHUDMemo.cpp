#include "MetroidPrime/ScriptObjects/CScriptHUDMemo.hpp"

#include "MetroidPrime/HUD/CSamusHud.hpp"

#include "Kyoto/Text/CStringTable.hpp"

CScriptHUDMemo::CScriptHUDMemo(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                               const CHUDMemoParms& parms, const EDisplayType disp, CAssetId msg,
                               const bool active)
: CEntity(uid, info, active, name)
, mParms(parms)
, mDispType(disp)
, mStringTableId(msg)
, mStringTable(msg == kInvalidAssetId ? rstl::optional_object_null()
                                         : rstl::optional_object< TLockedToken< CStringTable > >(
                                               gpSimplePool->GetObj(SObjectTag('STRG', msg)))) {}

CScriptHUDMemo::~CScriptHUDMemo() {}

void CScriptHUDMemo::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_SetToZero:
    if (GetActive()) {
      if (mDispType == kDT_MessageBox) {
        mgr.ShowPausedHUDMemo(mStringTableId, mParms.GetDisplayTime());
      } else {
        if (mStringTable) {
          CSamusHud::DisplayHudMemo((*mStringTable)->GetString(0), mParms);
        } else {
          CSamusHud::DisplayHudMemo(rstl::wstring_l(L""), mParms);
        }
      }
    }
    break;
  case kSM_Deactivate:
    if (GetActive() && mDispType == kDT_StatusMessage) {
      CSamusHud::DisplayHudMemo(rstl::wstring_l(L""), CHUDMemoParms(0.f, false, true, false));
    }
    break;
  default:
    break;
  }

  CEntity::AcceptScriptMsg(msg, uid, mgr);
}

ENTITY_ACCEPT_IMPL(CScriptHUDMemo)
