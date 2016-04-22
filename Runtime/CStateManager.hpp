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

namespace urde
{
class CScriptMailbox;
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
class CWorld;
class CTexture;

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

    std::unique_ptr<CWorld> x850_world;

    /* Used to be a list of 32-element reserved_vectors */
    std::vector<TUniqueId> x858_objectGraveyard;

    // x86c_stateManagerContainer;
    std::unique_ptr<CCameraManager> x870_cameraManager;
    std::unique_ptr<CSortedListManager> x874_sortedListManager;
    std::unique_ptr<CWeaponMgr> x878_weaponManager;
    std::unique_ptr<CFluidPlaneManager> x87c_fluidPlaneManager;
    std::unique_ptr<CEnvFxManager> x880_envFxManager;
    std::unique_ptr<CActorModelParticles> x884_actorModelParticles;
    std::unique_ptr<CTeamAiTypes> x888_teamAiTypes;
    std::unique_ptr<CRumbleManager> x88c_rumbleManager;

    std::map<TGameScriptId, TUniqueId> x890_scriptIdMap;
    std::map<TEditorId, SScriptObjectStream> x8a4_loadedScriptObjects;

    std::shared_ptr<CPlayerState> x8b8_playerState;
    std::shared_ptr<CScriptMailbox> x8bc_scriptMailbox;
    std::shared_ptr<CMapWorldInfo> x8c0_mapWorldInfo;
    std::shared_ptr<CWorldTransManager> x8c4_worldTransManager;

    TAreaId x8c8_currentAreaId;
    TAreaId x8cc_nextAreaId;
    u32 x8d0_extFrameIdx = 0;
    u32 x8d4_updateFrameIdx = 0;
    u32 x8d8_drawFrameIdx = 0;

    std::vector<CLight> x8dc_dynamicLights;

    TLockedToken<CTexture> x8ec_shadowTex; /* DefaultShadow in MiscData */

    CRandom16 x8f8_random;
    CRandom16* x8fc_activeRandom = nullptr;

    FScriptLoader x904_loaderFuncs[int(EScriptObjectType::ScriptObjectTypeMAX)] = {};

    bool xab0_worldLoaded = false;

    std::set<std::string> xab4_uniqueInstanceNames;

    CCameraFilterPass xaf8_camFilterPasses[9];
    CCameraBlurPass xc88_camBlurPasses[9];

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

public:
    enum class EScriptPersistence
    {
    };

    CStateManager(const std::weak_ptr<CScriptMailbox>&,
                  const std::weak_ptr<CMapWorldInfo>&,
                  const std::weak_ptr<CPlayerState>&,
                  const std::weak_ptr<CWorldTransManager>&);

    void RenderLast(TUniqueId);
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
    void SendScriptMsg(TUniqueId uid, TEditorId eid, EScriptObjectMessage msg, EScriptObjectState state);
    void FreeScriptObjects(TAreaId);
    void GetBuildForScript(TEditorId) const;
    TEditorId GetEditorIdForUniqueId(TUniqueId) const;
    TUniqueId GetIdForScript(TEditorId) const;
    void GetIdListForScript(TEditorId) const;
    void LoadScriptObjects(TAreaId, CInputStream& in, EScriptPersistence);
    void LoadScriptObject(TAreaId, EScriptObjectType, u32, CInputStream& in, EScriptPersistence);
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
    void InitializeState(u32, TAreaId, u32);
    void CreateStandardGameObjects();
    CObjectList* ObjectListById(EGameObjectList type);
    const CObjectList* GetObjectListById(EGameObjectList type) const;
    void RemoveObject(TUniqueId);
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
    void AddObject(CEntity&, EScriptPersistence);
    void AddObject(CEntity*, EScriptPersistence);
    bool RayStaticIntersection(const zeus::CVector3f&, const zeus::CVector3f&, float,
                               const CMaterialFilter&) const;
    bool RayWorldIntersection(TUniqueId, const zeus::CVector3f&, const zeus::CVector3f&,
                              float, const CMaterialFilter&,
                              const rstl::reserved_vector<TUniqueId, 1024>& list) const;
    void UpdateObjectInLists(CEntity&);
    TUniqueId AllocateUniqueId();

    const std::shared_ptr<CPlayerState>& GetPlayerState() const {return x8b8_playerState;}
    CRandom16* GetActiveRandom() {return x8fc_activeRandom;}
    CRumbleManager& GetRumbleManager() {return *x88c_rumbleManager;}
    CCameraFilterPass& GetCameraFilterPass(int idx) {return xaf8_camFilterPasses[idx];}

    CWorld* GetWorld() {return x850_world.get();}

    std::shared_ptr<CMapWorldInfo> MapWorldInfo() { return x8c0_mapWorldInfo; }
};

}

#endif
