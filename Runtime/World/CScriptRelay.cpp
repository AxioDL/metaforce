#include "CScriptRelay.hpp"

namespace urde
{

CScriptRelay::CScriptRelay(TUniqueId uid, const std::string& name, const CEntityInfo& info, bool active)
    : CEntity(uid, info, active, name)
{
}

void CScriptRelay::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager &stateMgr)
{
    CEntity::AcceptScriptMsg(msg, objId, stateMgr);
}

}
