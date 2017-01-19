#include "CScriptRelay.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde
{

CScriptRelay::CScriptRelay(TUniqueId uid, const std::string& name, const CEntityInfo& info, bool active)
    : CEntity(uid, info, active, name)
{
}

void CScriptRelay::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptRelay::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager &stateMgr)
{
    CEntity::AcceptScriptMsg(msg, objId, stateMgr);
    if (msg == EScriptObjectMessage::InternalMessage12)
    {
        UpdateObjectRef(stateMgr);
    }
    else if (msg == EScriptObjectMessage::SetToZero)
    {
        if (x30_24_active)
            return;

        x38_refCount++;
        TUniqueId tmp = stateMgr.GetLastRelayId();
        while (tmp != kInvalidUniqueId)
        {
            const CEntity* obj = stateMgr.GetObjectById(tmp);
            if (!obj)
            {
                tmp = x34_nextRelay;
                continue;
            }

            if (obj->GetUniqueId() == tmp)
                break;
        }

        if (tmp == kInvalidUniqueId)
            return;
        x34_nextRelay = stateMgr.GetLastRelayId();
        stateMgr.SetLastRelayId(GetUniqueId());
    }
}

void CScriptRelay::Think(float, CStateManager& stateMgr)
{
    if (x38_refCount == 0)
        return;

    while (x38_refCount != 0)
    {
        x38_refCount--;
        SendScriptMsgs(EScriptObjectState::Zero, stateMgr, EScriptObjectMessage::None);
    }
    UpdateObjectRef(stateMgr);
}

void CScriptRelay::UpdateObjectRef(CStateManager& stateMgr)
{
    TUniqueId* tmp = stateMgr.GetLastRelayIdPtr();
    while (*tmp != kInvalidUniqueId && tmp != nullptr)
    {
        if (*tmp == GetUniqueId())
        {
            *tmp = x34_nextRelay;
            return;
        }
        CScriptRelay* obj = dynamic_cast<CScriptRelay*>(stateMgr.ObjectById(*tmp));
        if (obj == nullptr)
            return;
        tmp = &obj->x34_nextRelay;
    }
}

}
