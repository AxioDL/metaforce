#include "CScriptHUDMemo.hpp"

namespace urde
{

CScriptHUDMemo::CScriptHUDMemo(TUniqueId uid, const std::string& name, const CEntityInfo& info, const CHUDMemoParms&,
                               CScriptHUDMemo::EDisplayType, ResId, bool active)
    : CEntity(uid, info, active, name)
{
}

}
