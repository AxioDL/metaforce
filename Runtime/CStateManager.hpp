#ifndef __URDE_CSTATEMANAGER_HPP__
#define __URDE_CSTATEMANAGER_HPP__

#include <memory>
#include "CBasics.hpp"
#include "ScriptObjectSupport.hpp"

namespace urde
{
class CScriptMailbox;
class CMapWorldInfo;
class CPlayerState;
class CWorldTransManager;

class CStateManager
{
    std::shared_ptr<CPlayerState> x8b8_playerState;
public:
    CStateManager(const std::weak_ptr<CScriptMailbox>&,
                  const std::weak_ptr<CMapWorldInfo>&,
                  const std::weak_ptr<CPlayerState>&,
                  const std::weak_ptr<CWorldTransManager>&);

    const std::shared_ptr<CPlayerState>& GetPlayerState() const {return x8b8_playerState;}

    void GetObjectListById() const
    {
    }
    void GetObjectById(TUniqueId uid) const
    {

    }
    void SendScriptMsg(TUniqueId uid, TEditorId eid, EScriptObjectMessage msg, EScriptObjectState state);
};

}

#endif
