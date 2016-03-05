#include "CEntity.hpp"
#include "CStateManager.hpp"

namespace urde
{

CEntity::CEntity(TUniqueId uniqueId, const CEntityInfo& info, bool active)
: m_uid(uniqueId), m_info(info), m_active(active) {}

void CEntity::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr)
{
    switch (msg)
    {
    case EScriptObjectMessage::Activate:
        if (!GetActive())
        {
            SetActive(true);
            SendScriptMsgs(EScriptObjectState::Active, stateMgr, EScriptObjectMessage::None);
        }
        break;
    case EScriptObjectMessage::Deactivate:
        if (GetActive())
        {
            SetActive(false);
            SendScriptMsgs(EScriptObjectState::Inactive, stateMgr, EScriptObjectMessage::None);
        }
        break;
    case EScriptObjectMessage::ToggleActive:
        if (GetActive())
        {
            SetActive(false);
            SendScriptMsgs(EScriptObjectState::Inactive, stateMgr, EScriptObjectMessage::None);
        }
        else
        {
            SetActive(true);
            SendScriptMsgs(EScriptObjectState::Active, stateMgr, EScriptObjectMessage::None);
        }
        break;
    default: break;
    }
}

void CEntity::SendScriptMsgs(EScriptObjectState state, CStateManager& stateMgr, EScriptObjectMessage skipMsg)
{
    for (const SConnection& conn : m_info.m_conns)
        if (conn.state == state && conn.msg != skipMsg)
            stateMgr.SendScriptMsg(m_uid, conn.objId, conn.msg, state);
}

}
