#ifndef __URDE_CSTATEMANAGER_HPP__
#define __URDE_CSTATEMANAGER_HPP__

#include <memory>
#include "CBasics.hpp"
#include "ScriptObjectSupport.hpp"
#include "GameObjectLists.hpp"
#include "Camera/CCameraManager.hpp"

namespace urde
{
class CScriptMailbox;
class CMapWorldInfo;
class CPlayerState;
class CWorldTransManager;
class CObjectList;

class CStateManager
{
    TUniqueId x8_idArr[1024] = {};
    std::unique_ptr<CObjectList> x80c_allObjs;
    std::unique_ptr<CActorList> x814_allObjs;
    std::unique_ptr<CPhysicsActorList> x81c_allObjs;
    std::unique_ptr<CGameCameraList> x824_allObjs;
    std::unique_ptr<CGameLightList> x82c_allObjs;
    std::unique_ptr<CListeningAiList> x834_allObjs;
    std::unique_ptr<CAiWaypointList> x83c_allObjs;
    std::unique_ptr<CPlatformAndDoorList> x844_allObjs;
    std::list<u32> x858_;

    // x86c_stateManagerContainer;



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
    TUniqueId AllocateUniqueId();
};

}

#endif
