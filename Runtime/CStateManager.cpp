#include "CStateManager.hpp"

namespace urde
{

CStateManager::CStateManager(const std::weak_ptr<CScriptMailbox>&,
                             const std::weak_ptr<CMapWorldInfo>&,
                             const std::weak_ptr<CPlayerState>&,
                             const std::weak_ptr<CWorldTransManager>&)
{
}

void CStateManager::SendScriptMsg(TUniqueId uid, TEditorId eid, EScriptObjectMessage msg, EScriptObjectState state)
{
}

}
