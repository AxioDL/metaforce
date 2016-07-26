#include "CScriptRelay.hpp"
#include "CStateManager.hpp"

namespace urde
{

CScriptRelay::CScriptRelay(TUniqueId uid, const std::string& name, const CEntityInfo& info, bool active)
    : CEntity(uid, info, active, name)
{
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

        x38_++;
        TUniqueId tmp = stateMgr.xf76_;
        while (tmp != kInvalidUniqueId)
        {
            const CEntity* obj = stateMgr.GetObjectById(tmp);
            if (!obj)
            {
                tmp = x34_;
                continue;
            }

            if (obj->GetUniqueId() == tmp)
                break;
        }

        if (tmp == kInvalidUniqueId)
            return;
        x34_ = stateMgr.xf76_;
        stateMgr.xf76_ = GetUniqueId();
    }
}

void CScriptRelay::Think(float, CStateManager& stateMgr)
{
    if (x38_ == 0)
        return;

    while (x38_ != 0)
    {
        x38_--;
        SendScriptMsgs(EScriptObjectState::Zero, stateMgr, EScriptObjectMessage::None);
    }
    UpdateObjectRef(stateMgr);
}

void CScriptRelay::UpdateObjectRef(CStateManager& stateMgr)
{
    TUniqueId* tmp = &stateMgr.xf76_;
    while (*tmp != kInvalidUniqueId && tmp != nullptr)
    {
        if (*tmp == GetUniqueId())
        {
            *tmp = x34_;
            return;
        }
        const CScriptRelay* obj = dynamic_cast<const CScriptRelay*>(stateMgr.GetObjectById(*tmp));
        if (obj)
            tmp = (TUniqueId*)&obj->x34_;
    }
}

}
