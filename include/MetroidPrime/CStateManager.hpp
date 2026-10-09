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
  EScriptObjectType mType;
  unsigned long mPosition;
  unsigned long mLength;

  SScriptObjectStream(EScriptObjectType type, unsigned long position, unsigned long length)
  : mType(type), mPosition(position), mLength(length) {}
};

class CStateManager : public TOneStatic< CStateManager > {
public:
  typedef rstl::multimap< TEditorId, TUniqueId > TIdList;
  typedef rstl::pair< TIdList::const_iterator, TIdList::const_iterator > TIdListResult;

  enum EGameState { kGS_Running, kGS_SoftPaused, kGS_Paused };

  struct SAchievementInfo {
    enum EDisplayType { kDT_HudMemo, kDT_Subtitle, kDT_None };
    enum ECinematicMode { kCM_Disallow, kCM_Allow };

    // Enum identities are known from SEL exports; their full value sets remain unknown.
    int mAchievement; // NBonusCreditState::EBonusAchievement
    int mBonusCredit; // CTrilogyState::EBonusCredit
    const char* mMessageName;
    EDisplayType mDisplayType;
    float mDelay;
    float mDisplayTime;
    ECinematicMode mCinematicMode;

    SAchievementInfo(int achievement, const char* messageName, int bonusCredit,
                     EDisplayType displayType, float delay, float displayTime,
                     ECinematicMode cinematicMode)
    : mAchievement(achievement)
    , mBonusCredit(bonusCredit)
    , mMessageName(messageName)
    , mDisplayType(displayType)
    , mDelay(delay)
    , mDisplayTime(displayTime)
    , mCinematicMode(cinematicMode) {}
  };

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

#if VERSION >= VERSION_GM8P_00
  void SetProjectedShadow(CProjectedShadow* shadow);
#else
  void SetProjectedShadow(CProjectedShadow* shadow) { mProjectedShadow = shadow; }
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
  void SetViewportScaleX(float scale) const { mViewportScaleX = scale; }
  void SetViewportScaleY(float scale) const { mViewportScaleY = scale; }
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
  void EarnAchievementAndNotify(const SAchievementInfo& info);

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
  TAreaId GetNextAreaId() const { return mNextAreaId; }
  void SetCurrentAreaId(TAreaId);
  TAreaId GetVisAreaId() const;

  bool CanCreateProjectile(TUniqueId, EWeaponType, int) const;

  CSinglePathMaze* SinglePathMaze();
  const CSinglePathMaze* GetSinglePathMaze() const;
  void SetSinglePathMaze(rstl::single_ptr< CSinglePathMaze > maze);

  CPlayer* Player() { return mPlayer; }
#if !defined(TARGET_PC) && defined(CSTATEMANAGER_OUT_OF_LINE_GETPLAYER)
  const CPlayer* GetPlayer() const;
#else
  const CPlayer* GetPlayer() const { return mPlayer; }
#endif
  CCameraManager* CameraManager() { return mCameraManager; }
  const CCameraManager* GetCameraManager() const { return mCameraManager; }
  TUniqueId GetPlayerActorHead() const { return mPlayerActorHead; }
  void SetPlayerActorHead(TUniqueId id) { mPlayerActorHead = id; }
  CPlayerState* PlayerState() { return &*mPlayerState; }
  CPlayerState* GetPlayerState() const { return &*mPlayerState; }
  rstl::rc_ptr< CScriptLayerManager >& WorldLayerState() { return mWorldLayerState; }
  CWorld* World() { return mWorld.get(); }
  const CWorld* GetWorld() const { return mWorld.get(); }
  CScriptMailbox* Mailbox() { return mMailbox.GetPtr(); }
  const CScriptMailbox* GetMailbox() const { return mMailbox.GetPtr(); }
  CActorModelParticles* ActorModelParticles() { return mActorModelParticles; }
  const CActorModelParticles* GetActorModelParticles() const { return mActorModelParticles; }
  CEnvFxManager* EnvFxManager() { return mEnvFxManager; }
  const CEnvFxManager* GetEnvFxManager() const { return mEnvFxManager; }
  CRumbleManager* GetRumbleManager() { return mRumbleManager; }
  CFluidPlaneManager* FluidPlaneManager() { return mFluidPlaneManager; }
  const CFluidPlaneManager* GetFluidPlaneManager() const { return mFluidPlaneManager; }
  CWorldTransManager* WorldTransManager() { return mWorldTransManager.GetPtr(); }
  const CWorldTransManager* GetWorldTransManager() const { return mWorldTransManager.GetPtr(); }
  EGameState GetGameState() const { return mGameState; }
  void SetGameState(EGameState state);

  CRandom16* Random() const { return mRandom; }
  rstl::list< TUniqueId >& ActiveParasites() { return mActiveParasites; }
  void SetRandomAvailable(bool available) { mRandom = available ? &mRandomValue : nullptr; }
  uint GetUpdateFrameIndex() const { return mUpdateFrameIdx; }

  CObjectList& ObjectListById(EGameObjectList id) { return *mObjectLists[id]; }
  const CObjectList& GetObjectListById(EGameObjectList id) const { return *mObjectLists[id]; }

  void RemoveObject(TUniqueId);

  const CFinalInput& GetFinalInput() const { return mFinalInput; }

  CCameraFilterPass& CameraFilterPass(ECameraFilterStage stage) {
    return mCamFilterPasses[size_t(stage)];
  }
  const CCameraFilterPass& GetCameraFilterPass(ECameraFilterStage stage) const {
    return mCamFilterPasses[size_t(stage)];
  }

  CCameraBlurPass& CameraBlurPass(ECameraFilterStage idx) { return mCamBlurPasses[idx]; }

  const CCameraBlurPass& GetCameraBlurPass(ECameraFilterStage idx) const {
    return mCamBlurPasses[idx];
  }

  float GetThermalColdScale1() const { return mThermColdScale1; }
  float GetThermalColdScale2() const { return mThermColdScale2; }
  void SetThermalColdScale2(float s) { mThermColdScale2 = s; }
  // TODO ?
  void AddThermalColdScale2(float s) { mThermColdScale2 += s; }

  const bool IsGeneratingObject() const { return mGeneratingObject; }
  void SetIsGeneratingObject(bool gen) { mGeneratingObject = gen; }

  void ApplyDamageToWorld(TUniqueId, const CActor&, const CVector3f&, const CDamageInfo& info,
                          const CMaterialFilter&);
  bool ApplyLocalDamage(const CVector3f&, const CVector3f&, CActor&, float, const CWeaponMode&);
  void ApplyDamage(const TUniqueId damagerId, const TUniqueId damageeId,
                   const TUniqueId radiusSender, const CDamageInfo& info,
                   const CMaterialFilter& filter,
                   const CVector3f& knockbackVec = CVector3f::Zero());

  void InformListeners(const CVector3f&, const EListenNoiseType);

  // Fog
  void SetupFogForArea3XRange(TAreaId area) const;
  void SetupFogForArea3XRange(const CGameArea& area) const;
  void SetupFogForArea(TAreaId area) const;
  void SetupFogForArea(const CGameArea&) const;
  bool SetupFogForDraw() const;

  //
  void ShowPausedHUDMemo(CAssetId strg, float time);
  void QueueMessage(int frameCount, CAssetId msg, float f1);
  int GetHUDMessageFrameCount() const { return mHudMessageFrameCount; }
  float GetHUDMessageTime() const { return mHudMessageTime; }
  void IncrementHUDMessageFrameCounter() { ++mHudMessageFrameCount; }

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
  EStateManagerTransition GetDeferredStateTransition() const { return mDeferredTransition; }
  bool IsFullyInitialized() const { return mInitPhase == kIP_Done; }
  void EnterMapScreen() { DeferStateTransition(kSMT_MapScreen); }
  void EnterPauseScreen() { DeferStateTransition(kSMT_PauseGame); }
  void EnterLogBookScreen() { DeferStateTransition(kSMT_LogBook); }
  void EnterSaveGameScreen() { DeferStateTransition(kSMT_SaveGame); }
  void EnterMessageScreen(uint, float);
  bool GetWantsToEnterMapScreen() const { return mDeferredTransition == kSMT_MapScreen; }
  bool GetWantsToEnterPauseScreen() const { return mDeferredTransition == kSMT_PauseGame; }
  bool GetWantsToEnterLogBookScreen() const { return mDeferredTransition == kSMT_LogBook; }
  bool GetWantsToEnterSaveGameScreen() const { return mDeferredTransition == kSMT_SaveGame; }
  bool GetWantsToEnterMessageScreen() const {
    return mDeferredTransition == kSMT_MessageScreen;
  }

  EThermalDrawFlag GetThermalDrawFlag() const { return mThermalFlag; }

  void SetLastTriggerId(TUniqueId uid) { mLastTrigger = uid; }
  TUniqueId GetLastTriggerId() const { return mLastTrigger; }

  void SetLastRelayId(const TUniqueId& uid) { mLastRelay = uid; }
  TUniqueId* GetLastRelayIdPtr() { return &mLastRelay; }
  TUniqueId GetLastRelayId() const { return mLastRelay; }

  TUniqueId GetBossId() const { return mBossId; }
  float GetTotalBossEnergy() const { return mTotalBossEnergy; }
  uint GetBossStringIdx() const { return mBossStringIdx; }
  void SetBossParams(TUniqueId bossId, float maxEnergy, uint stringIdx);
  void SetEnergyBarActorInfo(TUniqueId bossId, float maxEnergy, uint stringIdx) {
    SetBossParams(bossId, maxEnergy, stringIdx);
  }
  void SetPendingOnScreenTex(CAssetId texId, const CVector2i& extent, const CVector2i& offset);
  const SOnScreenTex& GetPendingScreenTex() const { return mPendingScreenTex; }
  float IntegrateVisorFog(float f) const;

  void TouchSky() const;
  void TouchPlayerActor() const;

  void QuitGame() { mQuitGame = true; }
  bool GetWantsToQuit() const { return mQuitGame; }
  bool SpecialSkipCinematic();
  void SetCinematicSkipObject(TUniqueId id) { mSkipCineSpecialFunc = id; }
  TUniqueId GetCinematicSkipObject() const { return mSkipCineSpecialFunc; }
  void SetCinematicPause(bool pause) { mCinematicPause = pause; }
  void SetInSaveUI(bool b) { mInSaveUI = b; }
  bool GetInSaveUI() const { return mInSaveUI; }
  void SetInMapScreen(bool b) { mInMapScreen = b; }
  bool GetInMapScreen() const { return mInMapScreen; }
  void SetIsFullThreat(bool v) { mFullThreat = v; }
  bool IsFullThreat() const { return mFullThreat; }
  const rstl::vector< CLight >& GetDynamicLightList() const { return mDynamicLights; }
  uint GetInputFrameIdx() const { return mInputFrameIdx; }
  CMapWorldInfo* MapWorldInfo() const { return mMapWorldInfo.GetPtr(); }

  CAssetId GetPauseHUDMessage() const { return mPauseHudMessage; }
  
  void AddActiveFlickerBat(const TUniqueId& uid) { mActiveFlickerBats.push_back(uid); }
  void RemoveActiveFlickerBat(const TUniqueId& uid) { mActiveFlickerBats.remove(uid); }

  rstl::list< TUniqueId >& GetActiveFlickerBats() { return mActiveFlickerBats; }

  static void ReflectionDrawer(void*, const CVector3f&);
  void CacheReflection();
  void DrawReflection(const CVector3f& point);
  void DrawSpaceWarp(const CVector3f& point, float strength) const;
  
  void UpdateActorInSortedLists(CActor& actor);

private:
  enum EInitPhase { kIP_LoadWorld, kIP_LoadFirstArea, kIP_Done };

  ushort mNextFreeIndex;
  rstl::reserved_vector< ushort, 1024 > mObjectIndexArray;
  rstl::reserved_vector< rstl::auto_ptr< CObjectList >, 8 > mObjectLists;
  CPlayer* mPlayer;
  rstl::single_ptr< CWorld > mWorld;
  rstl::list< rstl::reserved_vector< TUniqueId, 32 > > mGraveyard;
  rstl::single_ptr< CStateManagerContainer > mStateManagerContainer;
  CCameraManager* mCameraManager;
  SL::CSortedListManager* mSortedListManager;
  CWeaponMgr* mWeaponMgr;
  CFluidPlaneManager* mFluidPlaneManager;
  CEnvFxManager* mEnvFxManager;
  CActorModelParticles* mActorModelParticles;
  uint x888_;
  CRumbleManager* mRumbleManager;
  rstl::multimap< TEditorId, TUniqueId > mScriptIdMap;
  rstl::map< TEditorId, SScriptObjectStream > mLoadedScriptObjects;
  rstl::ncrc_ptr< CPlayerState > mPlayerState;
  rstl::ncrc_ptr< CScriptMailbox > mMailbox;
  rstl::ncrc_ptr< CMapWorldInfo > mMapWorldInfo;
  rstl::ncrc_ptr< CWorldTransManager > mWorldTransManager;
  rstl::ncrc_ptr< CScriptLayerManager > mWorldLayerState;

  TAreaId mNextAreaId;
  TAreaId mPrevAreaId;
  uint mInputFrameIdx;
  uint mUpdateFrameIdx;
  mutable uint mObjectDrawToken;

  rstl::vector< CLight > mDynamicLights;

  TCachedToken< CTexture > mShadowTex;
  CRandom16 mRandomValue;
  CRandom16* mRandom;

  EGameState mGameState;
  rstl::reserved_vector< FScriptLoader, int(kST_MAX) > mLoaderFuncs;
  EInitPhase mInitPhase;
  rstl::set< rstl::string > mUniqueInstanceNames;

  CFinalInput mFinalInput;
  rstl::reserved_vector< CCameraFilterPass, kCFS_Max > mCamFilterPasses;
  rstl::reserved_vector< CCameraBlurPass, kCFS_Max > mCamBlurPasses;
  int mHintIdx;
  uint mHintPeriods;
  SOnScreenTex mPendingScreenTex;
  CAssetId mPauseHudMessage;
  float mEscapeTimer;
  float mEscapeTotalTime;
  float mCurTimeMod900;
  TUniqueId mBossId;
  float mTotalBossEnergy;
  uint mBossStringIdx;
  float mThermColdScale1;
  float mThermColdScale2;
  mutable float mViewportScaleX;
  mutable float mViewportScaleY;
  mutable EThermalDrawFlag mThermalFlag;
  TUniqueId mSkipCineSpecialFunc;
  rstl::list< TUniqueId > mActiveFlickerBats;
  rstl::list< TUniqueId > mActiveParasites;
  TUniqueId mPlayerActorHead;
  rstl::single_ptr< CSinglePathMaze > mCurrentMaze;
  TUniqueId mLastTrigger;
  TUniqueId mLastRelay;
  float mHudMessageTime;
  CProjectedShadow* mProjectedShadow;
  uint mHudMessageFrameCount;
  CAssetId xf84_;
  CAssetId xf88_;
  float xf8c_;
  EStateManagerTransition mDeferredTransition;
  bool mReadyToRender : 1;
  bool mQuitGame : 1;
  bool mGeneratingObject : 1;
  bool mInMapScreen : 1;
  bool mInSaveUI : 1;
  bool mCinematicPause : 1;
  bool mFullThreat : 1;

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
  void ApplyRadiusDamage(const CActor&, const CVector3f&, TUniqueId, CActor&, const CDamageInfo&);
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
NESTED_CHECK_SIZEOF(CStateManager, SAchievementInfo, 0x1c)
// The Trilogy field layout is not yet reconstructed.
#if VERSION < VERSION_R3IJ_00
CHECK_SIZEOF(CStateManager, 0xf98)
#endif

#if defined(TARGET_PC)
extern CStateManager* gpStateManager;
#endif

#endif // _CSTATEMANAGER
