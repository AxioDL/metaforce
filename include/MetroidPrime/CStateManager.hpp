#ifndef _CSTATEMANAGER
#define _CSTATEMANAGER

extern const int gkPVSEnabled;

#include "types.h"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/TOneStatic.hpp"
#include "Kyoto/TToken.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/Cameras/CCameraBlurPass.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Enemies/EListenNoiseType.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/SOnScreenTex.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/map.hpp"
#include "rstl/multimap.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/set.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"

class CAABox;
class CActor;
class CProjectedShadow;
class CPlane;
class CGameArea;
class CPVSVisSet;
class CActorModelParticles;
class CCollisionResponseData;
class CCameraManager;
class CEnvFxManager;
class CFluidPlaneManager;
class CMapWorldInfo;
class CObjectList;
class CPlayer;
class CPlayerState;
class CRumbleManager;
class CScriptMailbox;
class CStateManagerContainer;
class CVector3f;
class CWeaponMgr;
class CWorld;
class CWorldTransManager;
class CEntity;
class CSinglePathMaze;
class CRayCastResult;
class CScriptLayerManager;
class CLight;
class CDamageInfo;
class CDamageVulnerability;
class CMRay;
class CTexture;
class CViewport;

namespace SL {
class CSortedListManager;
} // namespace SL

#define kMaxEntities 1024
typedef rstl::reserved_vector< TUniqueId, kMaxEntities > TEntityList;

enum EStateManagerTransition {
  kSMT_InGame,
  kSMT_MapScreen,
  kSMT_PauseGame,
  kSMT_LogBook,
  kSMT_SaveGame,
  kSMT_MessageScreen
};

enum EThermalDrawFlag {
  kTD_Hot,
  kTD_Cold,
  kTD_Bypass,
};

struct SScriptObjectStream {
  EScriptObjectType x0_type;
  unsigned long x4_position;
  unsigned long x8_length;

  SScriptObjectStream(EScriptObjectType type, unsigned long position, unsigned long length)
  : x0_type(type), x4_position(position), x8_length(length) {}
};

class CStateManager : public TOneStatic< CStateManager > {
public:
  typedef rstl::multimap< TEditorId, TUniqueId > TIdList;
  typedef rstl::pair< TIdList::const_iterator, TIdList::const_iterator > TIdListResult;

  enum EGameState { kGS_Running, kGS_SoftPaused, kGS_Paused };

  enum ECameraFilterStage {
    kCFS_Zero,
    kCFS_One,
    kCFS_Two,
    kCFS_Three,
    kCFS_Four,
    kCFS_Five,
    kCFS_Six,
    kCFS_Seven,
    kCFS_Eight,
    kCFS_Max,
  };

  CStateManager(const rstl::ncrc_ptr< CScriptMailbox >&, const rstl::ncrc_ptr< CMapWorldInfo >&,
                const rstl::ncrc_ptr< CPlayerState >&, const rstl::ncrc_ptr< CWorldTransManager >&,
                const rstl::ncrc_ptr< CScriptLayerManager >&);
  ~CStateManager();

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  void SetProjectedShadow(CProjectedShadow* shadow);
#else
  void SetProjectedShadow(CProjectedShadow* shadow) { xf7c_projectedShadow = shadow; }
#endif

  void PreRender();
  bool RenderLast(const TUniqueId&);
  void ResetEscapeSequenceTimer(float);
  float GetEscapeSequenceTimer() const;
  void SendScriptMsg(TUniqueId uid, TEditorId target, EScriptObjectMessage msg,
                     EScriptObjectState state);
  void DeliverScriptMsg(CEntity* ent, TUniqueId target, EScriptObjectMessage msg);
  void SendScriptMsgAlways(TUniqueId uid, TUniqueId src, EScriptObjectMessage msg);
  void RecursiveDrawTree(TUniqueId) const;
  void FreeScriptObjects(TAreaId);
  void LoadScriptObjects(TAreaId, CInputStream&, rstl::vector< TEditorId >&);
  rstl::pair< TEditorId, TUniqueId >
  LoadScriptObject(TAreaId, EScriptObjectType, unsigned int, CInputStream&);
  void AddDrawableActor(const CActor& actor, const CVector3f& pos, const CAABox& bounds) const;
  void AddDrawableActorPlane(const CActor& actor, const CPlane& plane,
                             const CAABox& bounds) const;
  void SetupParticleHook(const CActor& actor) const;
  void DeleteObjectRequest(TUniqueId uid);
  rstl::pair< TEditorId, TUniqueId > GenerateObject(const TEditorId& eid);
  void InitScriptObjects(const rstl::vector< TEditorId >& ids);
  void AddObject(CEntity*);
  void AddObject(CEntity&);
  bool HasWorld() const;
  void SetViewportScaleX(float scale) const { xf2c_viewportScaleX = scale; }
  void SetViewportScaleY(float scale) const { xf30_viewportScaleY = scale; }
  TUniqueId AllocateUniqueId();
  const rstl::string& HashInstanceName(CInputStream& in);
  bool SwapOutAllPossibleMemory();
  void FrameBegin(unsigned int);
  void InitializeState(unsigned int, TAreaId, unsigned int);
  void SwapOutTexturesToARAM(int, unsigned int);
  void UpdateGameState();
  void PostUpdatePlayer(float);
  void Update(float);
  void ProcessInput(const CFinalInput&);
  void ProcessPlayerInput();
  void UpdateAreaSounds();
  void FrameEnd();
  void UpdateObjectInLists(CEntity&);
  rstl::pair< int, int > CalculateScanCompletionRate() const;

  void BuildNearList(TEntityList& nearList, const CVector3f& pos, const CVector3f& dir, float mag,
                     const CMaterialFilter& filter = CMaterialFilter::skPassEverything,
                     const CActor* actor = nullptr) const;
  void BuildNearList(TEntityList& nearList, const CAABox&, const CMaterialFilter&,
                     const CActor*) const;
  bool RayCollideWorld(const CVector3f& start, const CVector3f& end,
                       const CMaterialFilter& filter, const CActor* damagee);
  bool RayCollideWorld(const CVector3f& start, const CVector3f& end, const TEntityList& nearList,
                       const CMaterialFilter& filter, const CActor* damagee) const;

  CRayCastResult RayStaticIntersection(const CVector3f& pos, const CVector3f& dir, float length,
                                       const CMaterialFilter& filter) const;
  CRayCastResult RayWorldIntersection(TUniqueId& idOut, const CVector3f& pos, const CVector3f& dir,
                                      float length, const CMaterialFilter& filter,
                                      const TEntityList& list) const;
  void BuildColliderList(TEntityList& out, const CActor& actor, const CAABox& aabb) const;
  void BuildDynamicLightListForWorld();

  CEntity* ObjectById(TUniqueId uid);
  const CEntity* GetObjectById(TUniqueId uid) const;
  void AreaUnloaded(TAreaId);
  void PrepareAreaUnload(TAreaId);
  void AreaLoaded(TAreaId);
  TEditorId GetEditorIdForUniqueId(TUniqueId) const;
  TUniqueId GetIdForScript(TEditorId eid) const;
  TIdListResult GetIdListForScript(TEditorId) const;

  void SetActorAreaId(CActor& actor, const TAreaId);
  TAreaId GetNextAreaId() const { return x8cc_nextAreaId; }
  void SetCurrentAreaId(TAreaId);
  TAreaId GetVisAreaId() const;

  bool CanCreateProjectile(TUniqueId, EWeaponType, int) const;

  CSinglePathMaze* SinglePathMaze();
  const CSinglePathMaze* GetSinglePathMaze() const;
  void SetSinglePathMaze(rstl::single_ptr< CSinglePathMaze > maze);

  CPlayer* Player() { return x84c_player; }
#ifdef CSTATEMANAGER_OUT_OF_LINE_GETPLAYER
  const CPlayer* GetPlayer() const;
#else
  const CPlayer* GetPlayer() const { return x84c_player; }
#endif
  CCameraManager* CameraManager() { return x870_cameraManager; }
  const CCameraManager* GetCameraManager() const { return x870_cameraManager; }
  TUniqueId GetPlayerActorHead() const { return xf6c_playerActorHead; }
  void SetPlayerActorHead(TUniqueId id) { xf6c_playerActorHead = id; }
  CPlayerState* PlayerState() { return &*x8b8_playerState; }
  CPlayerState* GetPlayerState() const { return &*x8b8_playerState; }
  rstl::rc_ptr< CScriptLayerManager >& WorldLayerState() { return x8c8_worldLayerState; }
  CWorld* World() { return x850_world.get(); }
  const CWorld* GetWorld() const { return x850_world.get(); }
  CScriptMailbox* Mailbox() { return x8bc_mailbox.GetPtr(); }
  const CScriptMailbox* GetMailbox() const { return x8bc_mailbox.GetPtr(); }
  CActorModelParticles* ActorModelParticles() { return x884_actorModelParticles; }
  const CActorModelParticles* GetActorModelParticles() const { return x884_actorModelParticles; }
  CEnvFxManager* EnvFxManager() { return x880_envFxManager; }
  const CEnvFxManager* GetEnvFxManager() const { return x880_envFxManager; }
  CRumbleManager* GetRumbleManager() { return x88c_rumbleManager; }
  CFluidPlaneManager* FluidPlaneManager() { return x87c_fluidPlaneManager; }
  const CFluidPlaneManager* GetFluidPlaneManager() const { return x87c_fluidPlaneManager; }
  CWorldTransManager* WorldTransManager() { return x8c4_worldTransManager.GetPtr(); }
  const CWorldTransManager* GetWorldTransManager() const { return x8c4_worldTransManager.GetPtr(); }
  EGameState GetGameState() const { return x904_gameState; }
  void SetGameState(EGameState state);

  CRandom16* Random() const { return x900_random; }
  rstl::list< TUniqueId >& ActiveParasites() { return xf54_activeParasites; }
  void SetRandomAvailable(bool available) { x900_random = available ? &x8fc_random : nullptr; }
  uint GetUpdateFrameIndex() const { return x8d8_updateFrameIdx; }

  CObjectList& ObjectListById(EGameObjectList id) { return *x808_objectLists[id]; }
  const CObjectList& GetObjectListById(EGameObjectList id) const { return *x808_objectLists[id]; }

  void RemoveObject(TUniqueId);

  const CFinalInput& GetFinalInput() const { return xb54_finalInput; }

  CCameraFilterPass& CameraFilterPass(ECameraFilterStage stage) {
    return xb84_camFilterPasses[size_t(stage)];
  }
  const CCameraFilterPass& GetCameraFilterPass(ECameraFilterStage stage) const {
    return xb84_camFilterPasses[size_t(stage)];
  }

  CCameraBlurPass& CameraBlurPass(ECameraFilterStage idx) { return xd14_camBlurPasses[idx]; }

  const CCameraBlurPass& GetCameraBlurPass(ECameraFilterStage idx) const {
    return xd14_camBlurPasses[idx];
  }

  float GetThermalColdScale1() const { return xf24_thermColdScale1; }
  float GetThermalColdScale2() const { return xf28_thermColdScale2; }
  void SetThermalColdScale2(float s) { xf28_thermColdScale2 = s; }
  // TODO ?
  void AddThermalColdScale2(float s) { xf28_thermColdScale2 += s; }

  const bool IsGeneratingObject() const { return xf94_26_generatingObject; }
  void SetIsGeneratingObject(bool gen) { xf94_26_generatingObject = gen; }

  void ApplyDamageToWorld(TUniqueId, const CActor&, const CVector3f&, const CDamageInfo& info,
                          const CMaterialFilter&);
  bool ApplyLocalDamage(const CVector3f&, const CVector3f&, CActor&, float, const CWeaponMode&);
  void ApplyDamage(const TUniqueId damagerId, const TUniqueId damageeId,
                   const TUniqueId radiusSender, const CDamageInfo& info,
                   const CMaterialFilter& filter,
                   const CVector3f& knockbackVec = CVector3f::Zero());

  void InformListeners(const CVector3f&, EListenNoiseType);

  // Fog
  void SetupFogForArea3XRange(TAreaId area) const;
  void SetupFogForArea3XRange(const CGameArea& area) const;
  void SetupFogForArea(TAreaId area) const;
  void SetupFogForArea(const CGameArea&) const;
  bool SetupFogForDraw() const;

  //
  void ShowPausedHUDMemo(CAssetId strg, float time);
  void QueueMessage(int frameCount, CAssetId msg, float f1);
  int GetHUDMessageFrameCount() const { return xf80_hudMessageFrameCount; }
  float GetHUDMessageTime() const { return xf78_hudMessageTime; }
  void IncrementHUDMessageFrameCounter() { ++xf80_hudMessageFrameCount; }

  // Weapon
  int GetWeaponIdCount(TUniqueId, EWeaponType);
  void RemoveWeaponId(TUniqueId, EWeaponType);
  void AddWeaponId(TUniqueId, EWeaponType);

  // Draw
  CFrustumPlanes SetupViewForDraw(const CViewport&) const;
  bool GetVisSetForArea(TAreaId, TAreaId, CPVSVisSet&) const;
  void ResetViewAfterDraw(const CViewport&, const CTransform4f&) const;
  void DrawWorld() const;
  void RenderCamerasAndAreaLights() const;
  void DrawE3DeathEffect() const;
  void DrawAdditionalFilters() const;
  void GetCharacterRenderMaskAndTarget(bool, int&, int&);
  void DoCollisionResponse(const CCollisionResponseData& colRespData,
                           const CRayCastResult& rayCast, TUniqueId uid,
                           const CWeaponMode& weaponMode, bool w1, bool b1);
  void DrawDebugStuff() const;

  // State transitions
  bool CanShowMapScreen();
  void DeferStateTransition(EStateManagerTransition t);
  EStateManagerTransition GetDeferredStateTransition() const { return xf90_deferredTransition; }
  bool IsFullyInitialized() const { return xb3c_initPhase == kIP_Done; }
  void EnterMapScreen() { DeferStateTransition(kSMT_MapScreen); }
  void EnterPauseScreen() { DeferStateTransition(kSMT_PauseGame); }
  void EnterLogBookScreen() { DeferStateTransition(kSMT_LogBook); }
  void EnterSaveGameScreen() { DeferStateTransition(kSMT_SaveGame); }
  void EnterMessageScreen(uint, float);
  bool GetWantsToEnterMapScreen() const { return xf90_deferredTransition == kSMT_MapScreen; }
  bool GetWantsToEnterPauseScreen() const { return xf90_deferredTransition == kSMT_PauseGame; }
  bool GetWantsToEnterLogBookScreen() const { return xf90_deferredTransition == kSMT_LogBook; }
  bool GetWantsToEnterSaveGameScreen() const { return xf90_deferredTransition == kSMT_SaveGame; }
  bool GetWantsToEnterMessageScreen() const {
    return xf90_deferredTransition == kSMT_MessageScreen;
  }

  EThermalDrawFlag GetThermalDrawFlag() const { return xf34_thermalFlag; }

  void SetLastTriggerId(TUniqueId uid) { xf74_lastTrigger = uid; }
  TUniqueId GetLastTriggerId() const { return xf74_lastTrigger; }

  void SetLastRelayId(const TUniqueId& uid) { xf76_lastRelay = uid; }
  TUniqueId* GetLastRelayIdPtr() { return &xf76_lastRelay; }
  TUniqueId GetLastRelayId() const { return xf76_lastRelay; }

  TUniqueId GetBossId() const { return xf18_bossId; }
  float GetTotalBossEnergy() const { return xf1c_totalBossEnergy; }
  uint GetBossStringIdx() const { return xf20_bossStringIdx; }
  void SetBossParams(TUniqueId bossId, float maxEnergy, uint stringIdx);
  void SetEnergyBarActorInfo(TUniqueId bossId, float maxEnergy, uint stringIdx) {
    SetBossParams(bossId, maxEnergy, stringIdx);
  }
  void SetPendingOnScreenTex(CAssetId texId, const CVector2i& extent, const CVector2i& offset);
  const SOnScreenTex& GetPendingScreenTex() const { return xef4_pendingScreenTex; }
  float IntegrateVisorFog(float f) const;

  void TouchSky() const;
  void TouchPlayerActor() const;

  void QuitGame() { xf94_25_quitGame = true; }
  bool GetWantsToQuit() const { return xf94_25_quitGame; }
  bool SpecialSkipCinematic();
  void SetCinematicSkipObject(TUniqueId id) { xf38_skipCineSpecialFunc = id; }
  TUniqueId GetCinematicSkipObject() const { return xf38_skipCineSpecialFunc; }
  void SetCinematicPause(bool pause) { xf94_29_cinematicPause = pause; }
  void SetInSaveUI(bool b) { xf94_28_inSaveUI = b; }
  bool GetInSaveUI() const { return xf94_28_inSaveUI; }
  void SetInMapScreen(bool b) { xf94_27_inMapScreen = b; }
  bool GetInMapScreen() const { return xf94_27_inMapScreen; }
  void SetIsFullThreat(bool v) { xf94_30_fullThreat = v; }
  bool IsFullThreat() const { return xf94_30_fullThreat; }
  const rstl::vector< CLight >& GetDynamicLightList() const { return x8e0_dynamicLights; }
  uint GetInputFrameIdx() const { return x8d4_inputFrameIdx; }
  CMapWorldInfo* MapWorldInfo() const { return x8c0_mapWorldInfo.GetPtr(); }

  CAssetId GetPauseHUDMessage() const { return xf08_pauseHudMessage; }
  
  void AddActiveFlickerBat(const TUniqueId& uid) { xf3c_activeFlickerBats.push_back(uid); }
  void RemoveActiveFlickerBat(const TUniqueId& uid) { xf3c_activeFlickerBats.remove(uid); }

  rstl::list< TUniqueId >& GetActiveFlickerBats() { return xf3c_activeFlickerBats; }

  static void ReflectionDrawer(void*, const CVector3f&);
  void CacheReflection();
  void DrawReflection(const CVector3f& point);
  void DrawSpaceWarp(const CVector3f& point, float strength) const;
  
  void UpdateActorInSortedLists(CActor& actor);

private:
  enum EInitPhase { kIP_LoadWorld, kIP_LoadFirstArea, kIP_Done };

  ushort x0_nextFreeIndex;
  rstl::reserved_vector< ushort, 1024 > x4_objectIndexArray;
  rstl::reserved_vector< rstl::auto_ptr< CObjectList >, 8 > x808_objectLists;
  CPlayer* x84c_player;
  rstl::single_ptr< CWorld > x850_world;
  rstl::list< rstl::reserved_vector< TUniqueId, 32 > > x854_graveyard;
  rstl::single_ptr< CStateManagerContainer > x86c_stateManagerContainer;
  CCameraManager* x870_cameraManager;
  SL::CSortedListManager* x874_sortedListManager;
  CWeaponMgr* x878_weaponMgr;
  CFluidPlaneManager* x87c_fluidPlaneManager;
  CEnvFxManager* x880_envFxManager;
  CActorModelParticles* x884_actorModelParticles;
  uint x888_;
  CRumbleManager* x88c_rumbleManager;
  rstl::multimap< TEditorId, TUniqueId > x890_scriptIdMap;
  rstl::map< TEditorId, SScriptObjectStream > x8a4_loadedScriptObjects;
  rstl::ncrc_ptr< CPlayerState > x8b8_playerState;
  rstl::ncrc_ptr< CScriptMailbox > x8bc_mailbox;
  rstl::ncrc_ptr< CMapWorldInfo > x8c0_mapWorldInfo;
  rstl::ncrc_ptr< CWorldTransManager > x8c4_worldTransManager;
  rstl::ncrc_ptr< CScriptLayerManager > x8c8_worldLayerState;

  TAreaId x8cc_nextAreaId;
  TAreaId x8d0_prevAreaId;
  uint x8d4_inputFrameIdx;
  uint x8d8_updateFrameIdx;
  mutable uint x8dc_objectDrawToken;

  rstl::vector< CLight > x8e0_dynamicLights;

  TCachedToken< CTexture > x8f0_shadowTex;
  CRandom16 x8fc_random;
  CRandom16* x900_random;

  EGameState x904_gameState;
  rstl::reserved_vector< FScriptLoader, int(kST_MAX) > x90c_loaderFuncs;
  EInitPhase xb3c_initPhase;
  rstl::set< rstl::string > xb40_uniqueInstanceNames;

  CFinalInput xb54_finalInput;
  rstl::reserved_vector< CCameraFilterPass, kCFS_Max > xb84_camFilterPasses;
  rstl::reserved_vector< CCameraBlurPass, kCFS_Max > xd14_camBlurPasses;
  int xeec_hintIdx;
  uint xef0_hintPeriods;
  SOnScreenTex xef4_pendingScreenTex;
  CAssetId xf08_pauseHudMessage;
  float xf0c_escapeTimer;
  float xf10_escapeTotalTime;
  float xf14_curTimeMod900;
  TUniqueId xf18_bossId;
  float xf1c_totalBossEnergy;
  uint xf20_bossStringIdx;
  float xf24_thermColdScale1;
  float xf28_thermColdScale2;
  mutable float xf2c_viewportScaleX;
  mutable float xf30_viewportScaleY;
  mutable EThermalDrawFlag xf34_thermalFlag;
  TUniqueId xf38_skipCineSpecialFunc;
  rstl::list< TUniqueId > xf3c_activeFlickerBats;
  rstl::list< TUniqueId > xf54_activeParasites;
  TUniqueId xf6c_playerActorHead;
  rstl::single_ptr< CSinglePathMaze > xf70_currentMaze;
  TUniqueId xf74_lastTrigger;
  TUniqueId xf76_lastRelay;
  float xf78_hudMessageTime;
  CProjectedShadow* xf7c_projectedShadow;
  uint xf80_hudMessageFrameCount;
  CAssetId xf84_;
  CAssetId xf88_;
  float xf8c_;
  EStateManagerTransition xf90_deferredTransition;
  bool xf94_24_readyToRender : 1;
  bool xf94_25_quitGame : 1;
  bool xf94_26_generatingObject : 1;
  bool xf94_27_inMapScreen : 1;
  bool xf94_28_inSaveUI : 1;
  bool xf94_29_cinematicPause : 1;
  bool xf94_30_fullThreat : 1;

  void UpdateThermalVisor();
  void UpdateHintState(float dt);
  void MovePlatforms(float dt);
  void MoveDoors(float dt);
  void CrossTouchActors();
  void Think(float dt);
  void PreThinkObjects(float dt);
  void UpdateRoomAcoustics(TAreaId areaId);
  void UpdateSortedLists();
  void KnockBackPlayer(CPlayer&, const CVector3f&, float, float);
  void ApplyKnockBack(CActor&, const CDamageInfo&, const CDamageVulnerability&, const CVector3f&,
                      float);
  void ProcessRadiusDamage(const CActor&, CActor&, TUniqueId, const CDamageInfo&,
                           const CMaterialFilter&);
  void ApplyRadiusDamage(const CActor&, const CVector3f&, CActor&, const CDamageInfo&);
  const bool TestRayDamage(const CVector3f&, const CActor&, const TEntityList&) const;
  const bool MultiRayCollideWorld(const CMRay&, const CMaterialFilter&) const;
  void TestBombHittingWater(const CActor&, const CVector3f&, CActor&);
  rstl::optional_object< CAABox > CalculateObjectBounds(const CActor&);
  const bool RayCollideWorldInternal(const CVector3f& start, const CVector3f& end,
                               const CMaterialFilter& filter, const TEntityList& nearList,
                               const CActor* damagee) const;
  void UpdateEscapeSequenceTimer(float dt);
  void CreateStandardGameObjects();
  rstl::pair< const SScriptObjectStream*, TEditorId > GetBuildForScript(TEditorId eid) const;
  void MurderScriptInstanceNames();
  void ClearGraveyard();
  static void RendererDrawCallback(const void*, const void*, int);
  static const bool MemoryAllocatorAllocationFailedCallback(const void*, unsigned int);
};
CHECK_SIZEOF(CStateManager, 0xf98)

#endif // _CSTATEMANAGER
