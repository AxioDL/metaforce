#include "CScriptHUDMemo.hpp"
#include "TCastTo.hpp"

namespace urde
{

CScriptHUDMemo::CScriptHUDMemo(TUniqueId uid, const std::string& name, const CEntityInfo& info, const CHUDMemoParms&,
                               CScriptHUDMemo::EDisplayType, CAssetId, bool active)
    : CEntity(uid, info, active, name)
{
}

void CScriptHUDMemo::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

}
