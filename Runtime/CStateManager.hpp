#ifndef __RETRO_CSTATEMANAGER_HPP__
#define __RETRO_CSTATEMANAGER_HPP__

#include <memory>
#include "CBasics.hpp"
#include "CScriptMailbox.hpp"
#include "CMapWorldInfo.hpp"
#include "CPlayerState.hpp"
#include "CWorldTransManager.hpp"
#include "ScriptObjectSupport.hpp"

namespace Retro
{

class CStateManager
{
public:
    CStateManager(const std::weak_ptr<CScriptMailbox>&,
                  const std::weak_ptr<CMapWorldInfo>&,
                  const std::weak_ptr<CPlayerState>&,
                  const std::weak_ptr<CWorldTransManager>&);

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
