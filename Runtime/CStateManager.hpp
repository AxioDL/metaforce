#ifndef __URDE_CSTATEMANAGER_HPP__
#define __URDE_CSTATEMANAGER_HPP__

#include <memory>
#include "CBasics.hpp"
#include "ScriptObjectSupport.hpp"
#include "GameObjectLists.hpp"
#include "Camera/CCameraManager.hpp"
#include "Camera/CCameraFilter.hpp"
#include "CRandom16.hpp"

namespace urde
{
class CScriptMailbox;
class CMapWorldInfo;
class CPlayerState;
class CWorldTransManager;
class CObjectList;
class CSortedListManager;
class CWeaponMgr;
class CFluidPlaneManager;
class CEnvFxManager;
class CActorModelParticles;
class CTeamAiTypes;
class CRumbleManager;

class CStateManager
{
    TUniqueId x8_idArr[1024] = {};
    std::unique_ptr<CObjectList> x80c_allObjs;
    std::unique_ptr<CActorList> x814_actorObjs;
    std::unique_ptr<CPhysicsActorList> x81c_physActorObjs;
    std::unique_ptr<CGameCameraList> x824_cameraObjs;
    std::unique_ptr<CGameLightList> x82c_lightObjs;
    std::unique_ptr<CListeningAiList> x834_listenAiObjs;
    std::unique_ptr<CAiWaypointList> x83c_aiWaypointObjs;
    std::unique_ptr<CPlatformAndDoorList> x844_platformAndDoorObjs;
    std::list<u32> x858_;

    // x86c_stateManagerContainer;
    std::unique_ptr<CCameraManager> x870_cameraManager;
    std::unique_ptr<CSortedListManager> x874_sortedListManager;
    std::unique_ptr<CWeaponMgr> x878_weaponManager;
    std::unique_ptr<CFluidPlaneManager> x87c_fluidPlaneManager;
    std::unique_ptr<CEnvFxManager> x880_envFxManager;
    std::unique_ptr<CActorModelParticles> x884_actorModelParticles;
    std::unique_ptr<CTeamAiTypes> x888_teamAiTypes;
    std::unique_ptr<CRumbleManager> x88c_rumbleManager;

    CRandom16 x8f8_random;
    CRandom16* x8fc_activeRandom = nullptr;

    std::shared_ptr<CPlayerState> x8b8_playerState;

    CCameraFilterPass xaf8_camFilterPasses[9];
    CCameraBlurPass xc88_camBlurPasses[9];

public:
    CStateManager(const std::weak_ptr<CScriptMailbox>&,
                  const std::weak_ptr<CMapWorldInfo>&,
                  const std::weak_ptr<CPlayerState>&,
                  const std::weak_ptr<CWorldTransManager>&);

    const std::shared_ptr<CPlayerState>& GetPlayerState() const {return x8b8_playerState;}

    const CObjectList* GetObjectListById(EGameObjectList type) const
    {
        const std::unique_ptr<CObjectList>* lists = &x80c_allObjs;
        return lists[int(type)].get();
    }
    CObjectList* GetObjectListById(EGameObjectList type)
    {
        std::unique_ptr<CObjectList>* lists = &x80c_allObjs;
        return lists[int(type)].get();
    }
    CEntity* GetObjectById(TUniqueId uid)
    {
        return x80c_allObjs->GetObjectById(uid);
    }
    void SendScriptMsg(TUniqueId uid, TEditorId eid, EScriptObjectMessage msg, EScriptObjectState state);
    TUniqueId AllocateUniqueId();
    CRandom16* GetActiveRandom() {return x8fc_activeRandom;}

    CRumbleManager& GetRumbleManager() {return *x88c_rumbleManager;}

    CCameraFilterPass& GetCameraFilterPass(int idx) {return xaf8_camFilterPasses[idx];}
};

}

#endif
