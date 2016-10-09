#ifndef __URDE_CSTATEMANAGER_HPP__
#define __URDE_CSTATEMANAGER_HPP__

#include <memory>
#include <set>
#include "CBasics.hpp"
#include "World/ScriptObjectSupport.hpp"
#include "GameObjectLists.hpp"
#include "Camera/CCameraManager.hpp"
#include "Camera/CCameraFilter.hpp"
#include "CRandom16.hpp"
#include "zeus/CAABox.hpp"
#include "CWeaponMgr.hpp"
#include "World/CAi.hpp"
#include "CToken.hpp"
#include "World/ScriptLoader.hpp"
#include "Input/CFinalInput.hpp"
#include "CSortedLists.hpp"
#include "CFluidPlaneManager.hpp"
#include "World/CEnvFxManager.hpp"
#include "World/CActorModelParticles.hpp"
#include "Input/CRumbleManager.hpp"

namespace urde
{
class CRelayTracker;
class CMapWorldInfo;
class CPlayerState;
class CWorldTransManager;
class CObjectList;
class CSortedListManager;
class CFluidPlaneManager;
class CEnvFxManager;
class CActorModelParticles;
class CTeamAiTypes;
class CRumbleManager;
class CActor;
class CLight;
class CDamageInfo;
class CMaterialFilter;
class CFinalInput;
class CPlayer;
class CWorld;
class CTexture;
class CWorldLayerState;

struct SScriptObjectStream
{
    CEntity* x0_obj;
    EScriptObjectType x4_type;
    u32 x8_position;
    u32 xc_length;
};

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

    std::unique_ptr<CPlayer> x84c_player;
    std::unique_ptr<CWorld> x850_world;

    /* Used to be a list of 32-element reserved_vectors */
    std::vector<TUniqueId> x858_objectGraveyard;

    struct CStateManagerContainer
    {
        CCameraManager x0_cameraManager;
        CSortedListManager x3c0_sortedListManager;
        CWeaponMgr xe3d8_weaponManager;
        CFluidPlaneManager xe3ec_fluidPlaneManager;
        CEnvFxManager xe510_envFxManager;
        CActorModelParticles xf168_actorModelParticles;
        CRumbleManager xf250_rumbleManager;
        u32 xf344_ = 0;
        u32 xf370_ = 0;
        u32 xf39c_ = 0;
    };
    std::unique_ptr<CStateManagerContainer> x86c_stateManagerContainer;
    CCameraManager* x870_cameraManager = nullptr;
    CSortedListManager* x874_sortedListManager = nullptr;
    CWeaponMgr* x878_weaponManager = nullptr;
    CFluidPlaneManager* x87c_fluidPlaneManager = nullptr;
    CEnvFxManager* x880_envFxManager = nullptr;
    CActorModelParticles* x884_actorModelParticles = nullptr;
    CRumbleManager* x88c_rumbleManager = nullptr;

    std::multimap<TEditorId, TUniqueId> x890_scriptIdMap;
    std::map<TEditorId, SScriptObjectStream> x8a4_loadedScriptObjects;

    std::shared_ptr<CPlayerState> x8b8_playerState;
    std::shared_ptr<CRelayTracker> x8bc_relayTracker;
    std::shared_ptr<CMapWorldInfo> x8c0_mapWorldInfo;
    std::shared_ptr<CWorldTransManager> x8c4_worldTransManager;
    std::shared_ptr<CWorldLayerState> x8c8_worldLayerState;

    TAreaId x8cc_nextAreaId = 0;
    TAreaId x8d0_prevAreaId = kInvalidAreaId;
    //u32 x8d0_extFrameIdx = 0;
    //u32 x8d4_updateFrameIdx = 0;
    //u32 x8d8_drawFrameIdx = 0;

    std::vector<CLight> x8dc_dynamicLights;

    TLockedToken<CTexture> x8f0_shadowTex; /* DefaultShadow in MiscData */
    CRandom16 x8fc_random;
    CRandom16* x900_activeRandom = nullptr;
    FScriptLoader x90c_loaderFuncs[int(EScriptObjectType::ScriptObjectTypeMAX)] = {};

    bool xab0_worldLoaded = false;

    std::set<std::string> xab4_uniqueInstanceNames;

    enum class InitPhase
    {
        LoadWorld,
        LoadFirstArea,
        Done
    } xb3c_initPhase;

    CFinalInput xb54_finalInput;
    CCameraFilterPass xb84_camFilterPasses[9];
    CCameraBlurPass xd14_camBlurPasses[9];

    s32 xe60_ = -1;
    zeus::CVector3f xe64_;

    TUniqueId xe70_ = kInvalidUniqueId;
    zeus::CVector3f xe74_ = {0.f, 1.f, 1.f};

    s32 xe80_ = 2;
    TUniqueId xe84_ = kInvalidUniqueId;

    union
    {
        struct
        {
            bool xe86_24_;
            bool xe86_25_;
            bool xe86_26_;
            bool xe86_27_;
            bool xe86_28_;
            bool xe86_29_;
        };
        u16 _dummy = 0;
    };
    s32 xef4_;
    zeus::CVector2i xef8_;
    zeus::CVector2i xf00_;
    float xf24_thermColdScale1 = 0.f;
    float xf28_thermColdScale2 = 0.f;
    float xf2c_ = 1.f;
    float xf30_ = 1.f;
    TUniqueId xf6c_playerActor;
    void UpdateThermalVisor();

public:
    /* TODO: Figure out what these are
     * Public for CScriptRelay
     */
    TUniqueId xf76_ = kInvalidUniqueId;
    /* TODO: Public for CFirstPersonCamera */
    u32 x904_;

    CStateManager(const std::weak_ptr<CRelayTracker>&,
                  const std::weak_ptr<CMapWorldInfo>&,
                  const std::weak_ptr<CPlayerState>&,
                  const std::weak_ptr<CWorldTransManager>&,
                  const std::weak_ptr<CWorldLayerState>&);

    bool RenderLast(TUniqueId);
    void AddDrawableActor(const CActor& actor, const zeus::CVector3f& vec, const zeus::CAABox& aabb) const;
    void SpecialSkipCinematic();
    void GetVisAreaId() const;
    void GetWeaponIdCount(TUniqueId, EWeaponType);
    void RemoveWeaponId(TUniqueId, EWeaponType);
    void AddWeaponId(TUniqueId, EWeaponType);
    void UpdateEscapeSequenceTimer(float);
    float GetEscapeSequenceTimer() const;
    void ResetEscapeSequenceTimer(float);
    void SetupParticleHook(const CActor& actor) const;
    void MurderScriptInstanceNames();
    const std::string* HashInstanceName(CInputStream& in);
    void SetActorAreaId(CActor& actor, TAreaId);
    void TouchSky() const;
    void TouchPlayerActor();
    void DrawSpaceWarp(const zeus::CVector3f&, float) const;
    void DrawReflection(const zeus::CVector3f&);
    void CacheReflection();
    bool CanCreateProjectile(TUniqueId, EWeaponType, int) const;
    const CGameLightList* GetDynamicLightList() const;
    void BuildDynamicLightListForWorld(std::vector<CLight>& listOut) const;
    void DrawDebugStuff() const;
    void RenderCamerasAndAreaLights() const;
    void DrawE3DeathEffect() const;
    void DrawAdditionalFilters() const;
    void DrawWorld() const;
    void SetupFogForArea(const CGameArea& area) const;
    void PreRender();
    void GetVisSetForArea(TAreaId, TAreaId) const;
    void RecursiveDrawTree(TUniqueId) const;
    void SendScriptMsg(CEntity* dest, TUniqueId src, EScriptObjectMessage msg);
    void SendScriptMsg(TUniqueId dest, TUniqueId src, EScriptObjectMessage msg);
    void SendScriptMsg(TUniqueId src, TEditorId dest,
                       EScriptObjectMessage msg, EScriptObjectState state);
    void SendScriptMsgAlways(TUniqueId dest, TUniqueId src, EScriptObjectMessage);
    void FreeScriptObjects(TAreaId);
    void GetBuildForScript(TEditorId) const;
    TEditorId GetEditorIdForUniqueId(TUniqueId) const;
    TUniqueId GetIdForScript(TEditorId) const;
    std::pair<std::multimap<TEditorId, TUniqueId>::const_iterator,
              std::multimap<TEditorId, TUniqueId>::const_iterator>
    GetIdListForScript(TEditorId) const;
    void LoadScriptObjects(TAreaId, CInputStream& in, std::vector<TEditorId>& idsOut);
    void LoadScriptObject(TAreaId, EScriptObjectType, u32, CInputStream& in);
    void InitScriptObjects(std::vector<TEditorId>& ids);
    void InformListeners(const zeus::CVector3f&, EListenNoiseType);
    void ApplyKnockBack(CActor& actor, const CDamageInfo& info,
                        const CDamageVulnerability&, const zeus::CVector3f&, float);
    void ApplyDamageToWorld(TUniqueId, const CActor&, const zeus::CVector3f&,
                            const CDamageInfo& info, const CMaterialFilter&);
    void ProcessRadiusDamage(const CActor&, CActor&, const zeus::CVector3f&,
                             const CDamageInfo& info, const CMaterialFilter&);
    void ApplyRadiusDamage(const CActor&, const zeus::CVector3f&, CActor&,
                           const CDamageInfo& info);
    void ApplyLocalDamage(const zeus::CVector3f&, const zeus::CVector3f&, CActor&, float,
                          const CWeaponMode&);
    void ApplyDamage(TUniqueId, TUniqueId, TUniqueId, const CDamageInfo& info,
                     const CMaterialFilter&);
    void UpdateAreaSounds();
    void FrameEnd();
    void ProcessPlayerInput();
    void ProcessInput(const CFinalInput& input);
    void Update(float dt);
    void UpdateGameState();
    void FrameBegin();
    void InitializeState(ResId mlvlId, TAreaId aid, ResId mreaId);
    void CreateStandardGameObjects();
    const std::unique_ptr<CObjectList>& GetObjectList() const { return x80c_allObjs; }
    CObjectList* ObjectListById(EGameObjectList type);
    const CObjectList* GetObjectListById(EGameObjectList type) const;
    void RemoveObject(TUniqueId);
    void RemoveActor(TUniqueId);
    void UpdateRoomAcoustics(TAreaId);
    void SetCurrentAreaId(TAreaId);
    void ClearGraveyard();
    void DeleteObjectRequest(TUniqueId);
    CEntity* ObjectById(TUniqueId uid);
    const CEntity* GetObjectById(TUniqueId uid) const;
    void AreaUnloaded(TAreaId);
    void PrepareAreaUnload(TAreaId);
    void AreaLoaded(TAreaId);
    void BuildNearList(rstl::reserved_vector<TUniqueId, 1024>& listOut,
                       const zeus::CVector3f&, const zeus::CVector3f&, float,
                       const CMaterialFilter&, const CActor*) const;
    void BuildColliderList(rstl::reserved_vector<TUniqueId, 1024>& listOut,
                           const CActor&, const zeus::CAABox&) const;
    void BuildNearList(rstl::reserved_vector<TUniqueId, 1024>& listOut,
                       const zeus::CAABox&, const CMaterialFilter&, const CActor*) const;
    void UpdateActorInSortedLists(CActor&);
    void UpdateSortedLists();
    zeus::CAABox CalculateObjectBounds(const CActor&);
    void AddObject(CEntity&);
    void AddObject(CEntity*);
    bool RayStaticIntersection(const zeus::CVector3f&, const zeus::CVector3f&, float,
                               const CMaterialFilter&) const;
    bool RayWorldIntersection(TUniqueId, const zeus::CVector3f&, const zeus::CVector3f&,
                              float, const CMaterialFilter&,
                              const rstl::reserved_vector<TUniqueId, 1024>& list) const;
    void UpdateObjectInLists(CEntity&);
    TUniqueId AllocateUniqueId();

    const std::shared_ptr<CPlayerState>& GetPlayerState() const {return x8b8_playerState;}
    CRandom16* GetActiveRandom() {return x900_activeRandom;}
    CRumbleManager& GetRumbleManager() {return *x88c_rumbleManager;}
    CCameraFilterPass& GetCameraFilterPass(int idx) {return xb84_camFilterPasses[idx];}

    CEnvFxManager* GetEnvFxManager() { return x880_envFxManager; }
    CWorld* GetWorld() {return x850_world.get();}
    CRelayTracker* GetRelayTracker() { return x8bc_relayTracker.get(); }
    CCameraManager* GetCameraManager() const { return x870_cameraManager; }

    const std::shared_ptr<CMapWorldInfo>& MapWorldInfo() const { return x8c0_mapWorldInfo; }
    const std::shared_ptr<CWorldLayerState>& LayerState() const { return x8c8_worldLayerState; }

    bool IsLayerActive(TAreaId area, int layerIdx) { return false; }

    CPlayer& GetPlayer() const { return *x84c_player; }
    CPlayer* Player() const { return x84c_player.get(); }

    CObjectList& GetAllObjectList() const { return *x80c_allObjs; }
    CActorList& GetActorObjectList() const { return *x814_actorObjs; }
    CPhysicsActorList& GetPhysicsActorObjectList() const { return *x81c_physActorObjs; }
    CGameCameraList& GetCameraObjectList() const { return *x824_cameraObjs; }
    CGameLightList& GetLightObjectList() const { return *x82c_lightObjs; }
    CListeningAiList& GetListeningAiObjectList() const { return *x834_listenAiObjs; }
    CAiWaypointList& GetAiWaypointObjectList() const { return *x83c_aiWaypointObjs; }
    CPlatformAndDoorList& GetPlatformAndDoorObjectList() const { return *x844_platformAndDoorObjs; }
};
}

#endif
