#include "CScriptHUDMemo.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "GuiSys/CStringTable.hpp"
#include "MP1/CSamusHud.hpp"
#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CScriptHUDMemo::CScriptHUDMemo(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                               const CHUDMemoParms& parms, EDisplayType disp, CAssetId msg, bool active)
: CEntity(uid, info, active, name), x34_parms(parms), x3c_dispType(disp), x40_stringTableId(msg) {
  if (msg.IsValid())
    x44_stringTable.emplace(g_SimplePool->GetObj({FOURCC('STRG'), msg}));
}

void CScriptHUDMemo::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptHUDMemo::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::SetToZero) {
    if (GetActive()) {
      if (x3c_dispType == EDisplayType::MessageBox)
        mgr.ShowPausedHUDMemo(x40_stringTableId, x34_parms.GetDisplayTime());
      else if (x3c_dispType == EDisplayType::StatusMessage) {
        if (x44_stringTable)
          MP1::CSamusHud::DisplayHudMemo((*x44_stringTable)->GetString(0), x34_parms);
        else
          MP1::CSamusHud::DisplayHudMemo(u"", x34_parms);
      }
    }
  } else if (msg == EScriptObjectMessage::Deactivate && GetActive() && x3c_dispType == EDisplayType::StatusMessage)
    MP1::CSamusHud::DisplayHudMemo(u"", CHUDMemoParms(0.f, false, true, false));

  CEntity::AcceptScriptMsg(msg, uid, mgr);
}

} // namespace urde
