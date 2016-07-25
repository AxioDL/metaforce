#include "CScriptRandomRelay.hpp"
#include "CStateManager.hpp"

namespace urde
{
CScriptRandomRelay::CScriptRandomRelay(TUniqueId uid, const std::string& name, const CEntityInfo& info, s32 connCount, s32 variance,
                                       bool clamp, bool active)
    : CEntity(uid, info, active, name),
      x34_connectionCount((clamp && connCount > 100) ? 100 : connCount),
      x38_variance(variance),
      x3c_clamp(clamp)
{
}

void CScriptRandomRelay::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr)
{
    CEntity::AcceptScriptMsg(msg, objId, stateMgr);
    if (msg == EScriptObjectMessage::SetToZero)
    {
        if (!x30_24_active)
            return;
        SendLocalScriptMsgs(EScriptObjectState::Zero, stateMgr);
    }
}

void CScriptRandomRelay::SendLocalScriptMsgs(EScriptObjectState state, CStateManager &stateMgr)
{
    if (state != EScriptObjectState::Zero)
    {
        SendScriptMsgs(state, stateMgr, EScriptObjectMessage::None);
        return;
    }

#if 0
    std::vector<std::pair<CEntity*, EScriptObjectMessage>> objs;
    objs.reserve(10);
    for (SConnection& conn : x20_conns)
    {
        const std::unique_ptr<CObjectList>& objList = stateMgr.GetObjectList();
    }
#endif
}
}
