#include "CEntity.hpp"

namespace Retro
{

CEntity::CEntity(TUniqueId uniqueId, const CEntityInfo& info, bool active)
: m_uid(uniqueId), m_info(info), m_active(active) {}

void CEntity::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr)
{
    switch (msg)
    {
    case MsgActivate:
        if (!GetActive())
        {
            SetActive(true);
            SendScriptMsgs(StActive, stateMgr, MsgNone);
        }
        break;
    case MsgDeactivate:
        if (GetActive())
        {
            SetActive(false);
            SendScriptMsgs(StInactive, stateMgr, MsgNone);
        }
        break;
    case MsgToggleActive:
        if (GetActive())
        {
            SetActive(false);
            SendScriptMsgs(StInactive, stateMgr, MsgNone);
        }
        else
        {
            SetActive(true);
            SendScriptMsgs(StActive, stateMgr, MsgNone);
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
