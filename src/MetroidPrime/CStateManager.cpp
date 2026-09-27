#define CSTATEMANAGER_OUT_OF_LINE_GETPLAYER
#include "MetroidPrime/CStateManager.hpp"
#undef CSTATEMANAGER_OUT_OF_LINE_GETPLAYER

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CDecalManager.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CFluidPlaneCPU.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CGameHintInfo.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CMapWorld.hpp"
#include "MetroidPrime/CMapWorldInfo.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CProjectedShadow.hpp"
#include "MetroidPrime/CRipple.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CScriptLayerManager.hpp"
#include "MetroidPrime/CScriptMailbox.hpp"
#include "MetroidPrime/CSimpleShadow.hpp"
#include "MetroidPrime/CStateManagerContainer.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CThardus.hpp"
#include "MetroidPrime/Enemies/CWallCrawlerSwarm.hpp"
#include "MetroidPrime/GameObjectLists.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Player/CWorldTransManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDock.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptMazeNode.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerActor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRoomAcoustics.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpawnPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpecialFunction.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CSnakeWeedSwarm.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"

#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionPrimitive.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/Basics/RAssertDolphin.hpp"
#include "Kyoto/CARAMManager.hpp"
#include "Kyoto/CARAMToken.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CTimeProvider.hpp"
#include "Kyoto/Graphics/CGraphicsPalette.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "Weapons/CCollisionResponseData.hpp"
#include "Weapons/CDecal.hpp"
#include "Weapons/CProjectileWeapon.hpp"
#include "WorldFormat/CPVSAreaSet.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

#include "Kyoto/CFrameDelayedKiller.hpp"

#if defined(TARGET_PC)
CStateManager* gpStateManager = nullptr;
#endif

const int gkPVSEnabled = 1;

namespace {
class area_sorter {
public:
  area_sorter(const CVector3f& ref, TAreaId aid) : reference(ref), visAreaId(aid) {}

  bool operator()(const CGameArea* a, const CGameArea* b) const;

private:
  CVector3f reference;
  TAreaId visAreaId;
};

bool area_sorter::operator()(const CGameArea* a, const CGameArea* b) const {
  TAreaId aId = a->GetId();
  TAreaId bId = b->GetId();
  if (aId == bId) {
    return false;
  }
  if (aId == visAreaId) {
    return false;
  }
  if (b->GetId() == visAreaId) {
    return true;
  }
  const float aDot = CVector3f::Dot(reference, a->GetAABB().GetCenterPoint());
  const float bDot = CVector3f::Dot(reference, b->GetAABB().GetCenterPoint());
  return aDot > bDot;
}

static inline const rstl::vector< CGameHintInfo::CGameHint >& GetGameHints() {
  return gpMemoryCard->GetHints();
}

static const float gkEpsilon = FLT_EPSILON;
static const float skBombUnderwaterRanges[2] = {2.f, 4.f};
static const float skBombAboveWaterRanges[2] = {1.f, 2.f};
static const float skBombUnderwaterMags[2] = {0.75f, 1.f};
static const float skBombAboveWaterMags[2] = {0.65f, 0.75f};
static float nextShake = 0.f;
static char init = 0;

} // namespace

static s64 sPreRenderStepTime;

class CLightPredicate {
public:
  bool operator()(const CLight& a, const CLight& b) const {
    if (a.GetPriority() > b.GetPriority()) {
      return true;
    } else if (a.GetPriority() == b.GetPriority()) {
      return a.GetIntensity() > b.GetIntensity();
    } else {
      return false;
    }
  }
};

CStateManager::CStateManager(const rstl::ncrc_ptr< CScriptMailbox >& mailbox,
                             const rstl::ncrc_ptr< CMapWorldInfo >& mwInfo,
                             const rstl::ncrc_ptr< CPlayerState >& playerState,
                             const rstl::ncrc_ptr< CWorldTransManager >& wtMgr,
                             const rstl::ncrc_ptr< CScriptLayerManager >& layerState)
: mNextFreeIndex(0)
, mObjectIndexArray(0)
, mObjectLists(rstl::auto_ptr< CObjectList >())

, mPlayer(nullptr)

, mStateManagerContainer(rs_new CStateManagerContainer())

, mCameraManager(&mStateManagerContainer->mCameraManager)
, mSortedListManager(&mStateManagerContainer->mSortedListManager)
, mWeaponMgr(&mStateManagerContainer->mWeaponManager)
, mFluidPlaneManager(&mStateManagerContainer->mFluidPlaneManager)
, mEnvFxManager(&mStateManagerContainer->mEnvFxManager)
, mActorModelParticles(&mStateManagerContainer->mActorModelParticles)
, mRumbleManager(&mStateManagerContainer->mRumbleManager)

, mPlayerState(playerState)
, mMailbox(mailbox)
, mMapWorldInfo(mwInfo)
, mWorldTransManager(wtMgr)
, mWorldLayerState(layerState)
, mNextAreaId(0)
, mPrevAreaId(kInvalidAreaId)
, mInputFrameIdx(0)
, mUpdateFrameIdx(0)
, mObjectDrawToken(0)

, mShadowTex(gpSimplePool->GetObj("DefaultShadow"))
, mRandomValue(0)
, mRandom(nullptr)
, mGameState(kGS_Running)
, mInitPhase(kIP_LoadWorld)

// based on map, uses the call with the count.
, mCamFilterPasses(CCameraFilterPass())

// TODO: should not be inlined, but the constructor above is inlined
, mCamBlurPasses(kCFS_Max, CCameraBlurPass())

, mHintIdx(-1)
, mHintPeriods(0)
, mPendingScreenTex()
, mPauseHudMessage(kInvalidAssetId)

, mEscapeTimer(0.0f)
, mEscapeTotalTime(0.0f)
, mCurTimeMod900(0.0f)
, mBossId(kInvalidUniqueId)
, mTotalBossEnergy(0.0f)

, mBossStringIdx(0)
, mThermColdScale1(0.0f)
, mThermColdScale2(0.0f)

, mViewportScaleX(1.f)
, mViewportScaleY(1.f)
, mThermalFlag(kTD_Bypass)

, mSkipCineSpecialFunc(kInvalidUniqueId)
, mPlayerActorHead(kInvalidUniqueId)
, mCurrentMaze(nullptr)
, mLastTrigger(kInvalidUniqueId)
, mLastRelay(kInvalidUniqueId)

, mHudMessageTime(0.0f)
, mProjectedShadow(nullptr)
, mHudMessageFrameCount(0)

, xf84_(kInvalidAssetId)
, xf88_(kInvalidAssetId)
, xf8c_(0.0f)
, mDeferredTransition(kSMT_InGame)

, mReadyToRender(false)
, mQuitGame(false)
, mGeneratingObject(false)
, mInMapScreen(false)
, mInSaveUI(false)
, mCinematicPause(false)
, mFullThreat(false)

{
  mObjectLists[0] = rs_new CObjectList(kOL_All);
  mObjectLists[1] = rs_new CActorList();
  mObjectLists[2] = rs_new CPhysicsActorList();
  mObjectLists[3] = rs_new CGameCameraList();
  mObjectLists[4] = rs_new CGameLightList();
  mObjectLists[5] = rs_new CListeningAiList();
  mObjectLists[6] = rs_new CAiWaypointList();
  mObjectLists[7] = rs_new CPlatformAndDoorList();

  gpRender->SetDrawableCallback(RendererDrawCallback, this);
  CMemory::SetOutOfMemoryCallback(MemoryAllocatorAllocationFailedCallback, this);

  mLoaderFuncs.resize(mLoaderFuncs.capacity());
  mLoaderFuncs[kST_Trigger] = ScriptLoader::LoadTrigger;
  mLoaderFuncs[kST_DamageableTrigger] = ScriptLoader::LoadDamageableTrigger;
  mLoaderFuncs[kST_Actor] = ScriptLoader::LoadActor;
  mLoaderFuncs[kST_Enemy] = ScriptLoader::LoadEnemy;
  mLoaderFuncs[kST_Waypoint] = ScriptLoader::LoadWaypoint;
  mLoaderFuncs[kST_Door] = ScriptLoader::LoadDoor;
  mLoaderFuncs[kST_Effect] = ScriptLoader::LoadEffect;
  mLoaderFuncs[kST_Timer] = ScriptLoader::LoadTimer;
  mLoaderFuncs[kST_Counter] = ScriptLoader::LoadCounter;
  mLoaderFuncs[kST_Sound] = ScriptLoader::LoadSound;
  mLoaderFuncs[kST_Platform] = ScriptLoader::LoadPlatform;
  mLoaderFuncs[kST_Generator] = ScriptLoader::LoadGenerator;
  mLoaderFuncs[kST_Dock] = ScriptLoader::LoadDock;
  mLoaderFuncs[kST_Camera] = ScriptLoader::LoadCamera;
  mLoaderFuncs[kST_CameraWaypoint] = ScriptLoader::LoadCameraWaypoint;
  mLoaderFuncs[kST_NewIntroBoss] = ScriptLoader::LoadNewIntroBoss;
  mLoaderFuncs[kST_SpawnPoint] = ScriptLoader::LoadSpawnPoint;
  mLoaderFuncs[kST_CameraHint] = ScriptLoader::LoadCameraHint;
  mLoaderFuncs[kST_CameraHintTrigger] = ScriptLoader::LoadCameraHintTrigger;
  mLoaderFuncs[kST_Pickup] = ScriptLoader::LoadPickup;
  mLoaderFuncs[kST_JumpPoint] = ScriptLoader::LoadJumpPoint;
  mLoaderFuncs[kST_MemoryRelay] = ScriptLoader::LoadRelay;
  mLoaderFuncs[kST_RandomRelay] = ScriptLoader::LoadRandomRelay;
  mLoaderFuncs[kST_Relay] = ScriptLoader::LoadFaultyRelay;

  mLoaderFuncs[kST_Beetle] = ScriptLoader::LoadBeetle;
  mLoaderFuncs[kST_HUDMemo] = ScriptLoader::LoadHUDMemo;
  mLoaderFuncs[kST_CameraFilterKeyframe] = ScriptLoader::LoadCameraFilterKeyframe;
  mLoaderFuncs[kST_CameraBlurKeyframe] = ScriptLoader::LoadCameraBlurKeyframe;
  mLoaderFuncs[kST_Debris] = ScriptLoader::LoadDebris;
  mLoaderFuncs[kST_CameraShaker] = ScriptLoader::LoadCameraShaker;
  mLoaderFuncs[kST_ActorKeyframe] = ScriptLoader::LoadActorKeyframe;
  mLoaderFuncs[kST_AIKeyframe] = ScriptLoader::LoadAIKeyframe;
  mLoaderFuncs[kST_Water] = ScriptLoader::LoadWater;
  mLoaderFuncs[kST_Steam] = ScriptLoader::LoadSteam;
  mLoaderFuncs[kST_Ripple] = ScriptLoader::LoadRipple;
  mLoaderFuncs[kST_Warwasp] = ScriptLoader::LoadWarWasp;
  mLoaderFuncs[kST_MapStation] = ScriptLoader::LoadMapStation;
  mLoaderFuncs[kST_SpacePirate] = ScriptLoader::LoadSpacePirate;
  mLoaderFuncs[kST_CoverPoint] = ScriptLoader::LoadCoverPoint;
  mLoaderFuncs[kST_SpiderBallWaypoint] = ScriptLoader::LoadSpiderBallWaypoint;
  mLoaderFuncs[kST_BloodFlower] = ScriptLoader::LoadBloodFlower;
  mLoaderFuncs[kST_PathCamera] = ScriptLoader::LoadPathCamera;
  mLoaderFuncs[kST_GrapplePoint] = ScriptLoader::LoadGrapplePoint;
  mLoaderFuncs[kST_FlickerBat] = ScriptLoader::LoadFlickerBat;
  mLoaderFuncs[kST_ChozoGhost] = ScriptLoader::LoadChozoGhost;
  mLoaderFuncs[kST_PuddleSpore] = ScriptLoader::LoadPuddleSpore;
  mLoaderFuncs[kST_DebugCameraWaypoint] = ScriptLoader::LoadDebugCameraWaypoint;
  mLoaderFuncs[kST_SpiderBallAttractionSurface] = ScriptLoader::LoadSpiderBallAttractionSurface;
  mLoaderFuncs[kST_PuddleToadGamma] = ScriptLoader::LoadPuddleToadGamma;
  mLoaderFuncs[kST_FireFlea] = ScriptLoader::LoadFireFlea;
  mLoaderFuncs[kST_DistanceFog] = ScriptLoader::LoadDistanceFog;
  mLoaderFuncs[kST_DockAreaChange] = ScriptLoader::LoadDockAreaChange;
  mLoaderFuncs[kST_ActorRotate] = ScriptLoader::LoadActorRotate;
  mLoaderFuncs[kST_SpecialFunction] = ScriptLoader::LoadSpecialFunction;
  mLoaderFuncs[kST_Metaree] = ScriptLoader::LoadMetaree;
  mLoaderFuncs[kST_SpankWeed] = ScriptLoader::LoadSpankWeed;
  mLoaderFuncs[kST_Parasite] = ScriptLoader::LoadParasite;
  mLoaderFuncs[kST_PlayerHint] = ScriptLoader::LoadPlayerHint;
  mLoaderFuncs[kST_Ripper] = ScriptLoader::LoadRipper;
  mLoaderFuncs[kST_PickupGenerator] = ScriptLoader::LoadPickupGenerator;
  mLoaderFuncs[kST_PointOfInterest] = ScriptLoader::LoadPointOfInterest;
  mLoaderFuncs[kST_Drone] = ScriptLoader::LoadDrone;
  mLoaderFuncs[kST_Metroid] = ScriptLoader::LoadMetroid;
  mLoaderFuncs[kST_DebrisExtended] = ScriptLoader::LoadDebrisExtended;
  mLoaderFuncs[kST_BallTrigger] = ScriptLoader::LoadBallTrigger;
  mLoaderFuncs[kST_TargetingPoint] = ScriptLoader::LoadTargetingPoint;
  mLoaderFuncs[kST_EMPulse] = ScriptLoader::LoadEMPulse;
  mLoaderFuncs[kST_IceSheegoth] = ScriptLoader::LoadIceSheegoth;
  mLoaderFuncs[kST_PlayerActor] = ScriptLoader::LoadPlayerActor;
  mLoaderFuncs[kST_Flaahgra] = ScriptLoader::LoadFlaahgra;
  mLoaderFuncs[kST_AreaAttributes] = ScriptLoader::LoadAreaAttributes;
  mLoaderFuncs[kST_FishCloud] = ScriptLoader::LoadFishCloud;
  mLoaderFuncs[kST_FishCloudModifier] = ScriptLoader::LoadFishCloudModifier;
  mLoaderFuncs[kST_VisorFlare] = ScriptLoader::LoadVisorFlare;
  mLoaderFuncs[kST_WorldTeleporter] = ScriptLoader::LoadWorldTeleporter;
  mLoaderFuncs[kST_VisorGoo] = ScriptLoader::LoadVisorGoo;
  mLoaderFuncs[kST_JellyZap] = ScriptLoader::LoadJellyZap;
  mLoaderFuncs[kST_ControllerAction] = ScriptLoader::LoadControllerAction;
  mLoaderFuncs[kST_Switch] = ScriptLoader::LoadSwitch;
  mLoaderFuncs[kST_PlayerStateChange] = ScriptLoader::LoadPlayerStateChange;
  mLoaderFuncs[kST_Thardus] = ScriptLoader::LoadThardus;
  mLoaderFuncs[kST_SaveStation] = ScriptLoader::LoadSaveStation;
  mLoaderFuncs[kST_WallCrawlerSwarm] = ScriptLoader::LoadWallCrawlerSwarm;
  mLoaderFuncs[kST_AIJumpPoint] = ScriptLoader::LoadAiJumpPoint;
  mLoaderFuncs[kST_FlaahgraTentacle] = ScriptLoader::LoadFlaahgraTentacle;
  mLoaderFuncs[kST_RoomAcoustics] = ScriptLoader::LoadRoomAcoustics;
  mLoaderFuncs[kST_FlyingPirate] = ScriptLoader::LoadFlyingPirate;
  mLoaderFuncs[kST_ColorModulate] = ScriptLoader::LoadColorModulate;
  mLoaderFuncs[kST_ThardusRockProjectile] = ScriptLoader::LoadThardusRockProjectile;
  mLoaderFuncs[kST_Midi] = ScriptLoader::LoadMidi;
  mLoaderFuncs[kST_StreamedAudio] = ScriptLoader::LoadStreamedMusic;
  mLoaderFuncs[kST_WorldTeleporterToo] = ScriptLoader::LoadWorldTeleporter;
  mLoaderFuncs[kST_Repulsor] = ScriptLoader::LoadRepulsor;
  mLoaderFuncs[kST_GunTurret] = ScriptLoader::LoadGunTurret;
  mLoaderFuncs[kST_FogVolume] = ScriptLoader::LoadFogVolume;
  mLoaderFuncs[kST_Babygoth] = ScriptLoader::LoadBabygoth;
  mLoaderFuncs[kST_Eyeball] = ScriptLoader::LoadEyeBall;
  mLoaderFuncs[kST_RadialDamage] = ScriptLoader::LoadRadialDamage;
  mLoaderFuncs[kST_CameraPitchVolume] = ScriptLoader::LoadScriptCameraPitchVolume;
  mLoaderFuncs[kST_ElitePirate] = ScriptLoader::LoadElitePirate;
  mLoaderFuncs[kST_MetroidBeta] = ScriptLoader::LoadMetroidBeta;
  mLoaderFuncs[kST_EnvFxDensityController] = ScriptLoader::LoadEnvFxDensityController;
  mLoaderFuncs[kST_Magdolite] = ScriptLoader::LoadMagdolite;
  mLoaderFuncs[kST_TeamAIMgr] = ScriptLoader::LoadTeamAiMgr;
  mLoaderFuncs[kST_SnakeWeedSwarm] = ScriptLoader::LoadSnakeWeedSwarm;
  mLoaderFuncs[kST_ActorContraption] = ScriptLoader::LoadActorContraption;
  mLoaderFuncs[kST_AtomicAlpha] = ScriptLoader::LoadAtomicAlpha;
  mLoaderFuncs[kST_Oculus] = ScriptLoader::LoadOcculus;
  mLoaderFuncs[kST_Geemer] = ScriptLoader::LoadGeemer;
  mLoaderFuncs[kST_SpindleCamera] = ScriptLoader::LoadSpindleCamera;
  mLoaderFuncs[kST_RumbleEffect] = ScriptLoader::LoadRumbleEffect;
  mLoaderFuncs[kST_AmbientAI] = ScriptLoader::LoadAmbientAI;
  mLoaderFuncs[0x76] = NULL;
  mLoaderFuncs[kST_AtomicBeta] = ScriptLoader::LoadAtomicBeta;
  mLoaderFuncs[kST_IceZoomer] = ScriptLoader::LoadIceZoomer;
  mLoaderFuncs[kST_Puffer] = ScriptLoader::LoadPuffer;
  mLoaderFuncs[kST_Tryclops] = ScriptLoader::LoadTryclops;
  mLoaderFuncs[kST_Ridley] = ScriptLoader::LoadRidley;
  mLoaderFuncs[kST_Seedling] = ScriptLoader::LoadSeedling;
  mLoaderFuncs[kST_ThermalHeatFader] = ScriptLoader::LoadThermalHeatFader;
  mLoaderFuncs[0x7e] = NULL;
  mLoaderFuncs[kST_Burrower] = ScriptLoader::LoadBurrower;
  mLoaderFuncs[kST_ScriptBeam] = ScriptLoader::LoadScriptBeam;
  mLoaderFuncs[0x80] = NULL;
  mLoaderFuncs[kST_MetroidPrimeStage1] = ScriptLoader::LoadMetroidPrimeRelay;
  mLoaderFuncs[0x1f] = NULL;
  mLoaderFuncs[0x23] = NULL;
  mLoaderFuncs[kST_WorldLightFader] = ScriptLoader::LoadWorldLightFader;
  mLoaderFuncs[kST_MetroidPrimeStage2] = ScriptLoader::LoadMetroidPrimeStage2;
  mLoaderFuncs[kST_MazeNode] = ScriptLoader::LoadMazeNode;
  mLoaderFuncs[kST_OmegaPirate] = ScriptLoader::LoadOmegaPirate;
  mLoaderFuncs[kST_PhazonPool] = ScriptLoader::LoadPhazonPool;
  mLoaderFuncs[kST_PhazonHealingNodule] = ScriptLoader::LoadPhazonHealingNodule;
  mLoaderFuncs[kST_NewCameraShaker] = ScriptLoader::LoadNewCameraShaker;
  mLoaderFuncs[kST_ShadowProjector] = ScriptLoader::LoadShadowProjector;
  mLoaderFuncs[kST_EnergyBall] = ScriptLoader::LoadEnergyBall;

  CGameCollision::InitCollision();
  CMemory::OffsetFakeStatics(mObjectLists.size() * sizeof(CObjectList) + 0x11c);
  ControlMapper::ResetCommandFilters();
  mShadowTex.Lock();
#if defined(TARGET_PC)
  gpStateManager = this;
#endif
}

CStateManager::~CStateManager() {
#if defined(TARGET_PC)
  gpStateManager = nullptr;
#endif
  CMemory::OffsetFakeStatics(
      -(mObjectLists.size() * sizeof(CObjectList) + 0x11c)); // TODO what is this 11c?
  mRumbleManager->HardStopAll();
  mEnvFxManager->Cleanup();

  mRandom = &mRandomValue;

  CObjectList& list = *mObjectLists[0];
  ClearGraveyard();
  for (int i = 0; i != kMaxObjects; ++i) {
    CEntity* ent = list[i];
    if (ent && ent->GetUniqueId() != mPlayer->GetUniqueId()) {
      ent->AcceptScriptMsg(kSM_Deleted, kInvalidUniqueId, *this);
      RemoveObject(ent->GetUniqueId());
      delete ent;
    }
  }
  ClearGraveyard();
  mPlayer->AcceptScriptMsg(kSM_Deleted, kInvalidUniqueId, *this);
  RemoveObject(mPlayer->GetUniqueId());
  delete mPlayer;
  mPlayer = nullptr;
  CCollisionPrimitive::Uninitialize();

  CMemory::SetOutOfMemoryCallback(nullptr, nullptr);
}

TUniqueId CStateManager::AllocateUniqueId() {
  const ushort lastIndex = mNextFreeIndex;
  ushort ourIndex;
  do {
    ourIndex = mNextFreeIndex;
    mNextFreeIndex = (ourIndex + 1) % 1024;
    if (mNextFreeIndex == lastIndex) {
      rs_debugger_printf("Object list full!");
    }
  } while (ObjectListById(kOL_All).GetObjectByIndex(ourIndex) != nullptr);

  mObjectIndexArray[ourIndex] = (mObjectIndexArray[ourIndex] + 1) & 0x3f;
  if (TUniqueId(mObjectIndexArray[ourIndex], ourIndex) == kInvalidUniqueId) {
    mObjectIndexArray[ourIndex] = 0;
  }

  return TUniqueId(mObjectIndexArray[ourIndex], ourIndex);
}

void CStateManager::UpdateObjectInLists(CEntity& ent) {
  rstl::auto_ptr< CObjectList >* listBegin = mObjectLists.data();
  for (rstl::auto_ptr< CObjectList >* listIt = listBegin;
       listIt != listBegin + mObjectLists.size(); ++listIt) {
    if (static_cast< const CObjectList* >(listIt->get())->GetValidObjectById(ent.GetUniqueId()) !=
            nullptr &&
        !listIt->get()->IsQualified(ent)) {
      listIt->get()->RemoveObject(ent.GetUniqueId());
    }

    if (static_cast< const CObjectList* >(listIt->get())->GetValidObjectById(ent.GetUniqueId()) ==
        nullptr) {
      listIt->get()->AddObject(ent);
    }
  }
}

CRayCastResult CStateManager::RayWorldIntersection(TUniqueId& idOut, const CVector3f& pos,
                                                   const CVector3f& dir, float length,
                                                   const CMaterialFilter& filter,
                                                   const TEntityList& list) const {
  return CGameCollision::RayWorldIntersection(*this, idOut, pos, dir, length, filter, list);
}

CRayCastResult CStateManager::RayStaticIntersection(const CVector3f& pos, const CVector3f& dir,
                                                    float length,
                                                    const CMaterialFilter& filter) const {
  return CGameCollision::RayStaticIntersection(*this, pos, dir, length, filter);
}

bool CStateManager::RayCollideWorld(const CVector3f& start, const CVector3f& end,
                                    const TEntityList& nearList, const CMaterialFilter& filter,
                                    const CActor* damagee) const {
  return RayCollideWorldInternal(start, end, filter, nearList, damagee);
}

bool CStateManager::RayCollideWorld(const CVector3f& start, const CVector3f& end,
                                    const CMaterialFilter& filter, const CActor* damagee) {
  TEntityList nearList;
  CVector3f delta = end - start;
  const float mag = delta.Magnitude();
  delta *= 1.f / mag;

  BuildNearList(nearList, start, delta, mag, filter, damagee);
  return RayCollideWorldInternal(start, end, filter, nearList, damagee);
}

const bool CStateManager::RayCollideWorldInternal(const CVector3f& start, const CVector3f& end,
                                                  const CMaterialFilter& filter,
                                                  const TEntityList& nearList,
                                                  const CActor* damagee) const {
  CVector3f delta = end - start;
  bool result = true;
  if (delta.CanBeNormalized()) {
    const float mag = delta.Magnitude();
    delta *= 1.f / mag;
    result = CGameCollision::RayStaticIntersectionBool(*this, start, delta, mag, filter);
    if (result) {
      result = CGameCollision::RayDynamicIntersectionBool(*this, start, delta, filter, nearList,
                                                          damagee, mag);
    }
  }
  return result;
}

bool CStateManager::HasWorld() const { return mWorld.get(); }

void CStateManager::AddObject(CEntity* obj) {
  if (!obj) {
    return;
  }
  AddObject(*obj);
}

void CStateManager::AddObject(CEntity& obj) {
  if (obj.GetEditorId() != kInvalidEditorId) {
    mScriptIdMap.insert(
        rstl::pair< TEditorId, TUniqueId >(obj.GetEditorId(), obj.GetUniqueId()));
  }

  rstl::reserved_vector< rstl::auto_ptr< CObjectList >, 8 >::iterator listIt =
      mObjectLists.begin();
  for (; listIt != mObjectLists.end(); ++listIt) {
    (*listIt)->AddObject(obj);
  }

  if (obj.mAreaId == kInvalidAreaId && mPlayer != nullptr &&
      obj.GetUniqueId() != mPlayer->GetUniqueId()) {
    obj.__SetCurrentAreaId(mPlayer->GetCurrentAreaId());
  }

  TAreaId objAreaId = obj.mAreaId;
  if (objAreaId != kInvalidAreaId) {
    CGameArea* area = mWorld->Area(objAreaId);
    if (area->IsPostConstructed()) {
      area->ObjectList()->AddObject(obj);
    }
  }

  if (CActor* actor = TCastToPtr< CActor >(obj)) {
    UpdateActorInSortedLists(*actor);
  }

  obj.AcceptScriptMsg(kSM_Registered, kInvalidUniqueId, *this);

  if (obj.mAreaId != kInvalidAreaId && HasWorld()) {
    CGameArea* area = mWorld->Area(obj.GetCurrentAreaId());
    if (area->IsValidated()) {
      DeliverScriptMsg(&obj, kInvalidUniqueId, kSM_InitializedInArea);
    }
  }
}

rstl::optional_object< CAABox > CStateManager::CalculateObjectBounds(const CActor& actor) {
  const CPhysicsActor* const physAct = TCastToConstPtr< CPhysicsActor >(actor);
  rstl::optional_object< CAABox > touchBounds = actor.GetTouchBounds();

  if (touchBounds) {
    const CAABox& touchBox = *touchBounds;
    CAABox aabb = CAABox::MakeMaxInvertedBox();
    aabb.AccumulateBounds(touchBox.GetMinPoint());
    aabb.AccumulateBounds(touchBox.GetMaxPoint());

    if (physAct != nullptr) {
      aabb.Include(physAct->GetBoundingBox());
    }

    return aabb;
  }

  if (physAct != nullptr) {
    return physAct->GetBoundingBox();
  }

  return rstl::optional_object_null();
}

void CStateManager::UpdateSortedLists() {
  if (mWorld.get() == nullptr) {
    return;
  }

  CObjectList* actorList = mObjectLists[kOL_Actor].get();
  for (int i = actorList->GetFirstObjectIndex(); i != -1; i = actorList->GetNextObjectIndex(i)) {
    CActor* actor = static_cast< CActor* >((*actorList)[i]);
    if (actor != nullptr) {
      UpdateActorInSortedLists(*actor);
    }
  }
}

void CStateManager::UpdateActorInSortedLists(CActor& actor) {
  if (actor.GetUseInSortedLists() && actor.GetTransformDirty()) {
    const rstl::optional_object< CAABox > bounds = CalculateObjectBounds(actor);
    const bool actorInLists = mSortedListManager->ActorInLists(&actor);
    const bool hasBounds = bounds.valid();
    if (actorInLists || hasBounds) {
      actor.SetTransformDirty(false);
      if (actorInLists) {
        if (!actor.GetActive() || !hasBounds) {
          mSortedListManager->Remove(&actor);
        } else {
          mSortedListManager->Move(&actor, *bounds);
        }
      } else if (actor.GetActive() && hasBounds) {
        mSortedListManager->Insert(&actor, *bounds);
      }
    }
  }
}

void CStateManager::BuildNearList(TEntityList& nearList, const CAABox& aabb,
                                  const CMaterialFilter& filter, const CActor* actor) const {
  mSortedListManager->BuildNearList(nearList, aabb, filter, actor);
}

void CStateManager::BuildColliderList(TEntityList& out, const CActor& actor,
                                      const CAABox& aabb) const {
  mSortedListManager->BuildNearList(out, actor, aabb);
}

void CStateManager::BuildNearList(TEntityList& nearList, const CVector3f& pos, const CVector3f& dir,
                                  float mag, const CMaterialFilter& filter,
                                  const CActor* actor) const {
  mSortedListManager->BuildNearList(nearList, pos, dir, mag, filter, actor);
}

void CStateManager::AreaLoaded(TAreaId aid) {
  mMailbox->SendMsgs(aid, *this);
  mEnvFxManager->AreaLoaded();
}

void CStateManager::PrepareAreaUnload(TAreaId aid) {
  CObjectList* allList = mObjectLists[0].get();
  for (int i = allList->GetFirstObjectIndex(); i != -1; i = allList->GetNextObjectIndex(i)) {
    if (CScriptDoor* const door = TCastToPtr< CScriptDoor >((*allList)[i])) {
      if (door->IsConnectedToArea(*this, aid)) {
        door->ForceClosed(*this);
      }
    }
  }

  FreeScriptObjects(aid);
}

void CStateManager::AreaUnloaded(TAreaId) {}

const CEntity* CStateManager::GetObjectById(TUniqueId uid) const {
  return GetObjectListById(kOL_All).GetObjectById(uid);
}

CEntity* CStateManager::ObjectById(TUniqueId uid) {
  return mObjectLists[0]->GetObjectById(uid);
}

void CStateManager::DeleteObjectRequest(TUniqueId uid) {
  CEntity* ent = ObjectById(uid);
  if (ent == nullptr) {
    return;
  }
  if (ent->IsInGraveyard()) {
    return;
  }

  ent->SetIsInGraveyard();

  if (mGraveyard.size() == 0) {
    rstl::reserved_vector< TUniqueId, 32 > newVec;
    mGraveyard.push_back(newVec);
  } else {
    rstl::list< rstl::reserved_vector< TUniqueId, 32 > >::iterator backIt = mGraveyard.end();
    --backIt;
    if (backIt->size() == backIt->capacity()) {
      rstl::reserved_vector< TUniqueId, 32 > newVec;
      mGraveyard.push_back(newVec);
    }
  }

  rstl::list< rstl::reserved_vector< TUniqueId, 32 > >::iterator backIt = mGraveyard.end();
  --backIt;
  backIt->push_back(uid);

  ent->AcceptScriptMsg(kSM_Deleted, kInvalidUniqueId, *this);
  ent->mScriptingBlocked = true;

  if (CActor* actor = TCastToPtr< CActor >(ent)) {
    mSortedListManager->Remove(actor);
    actor->SetUseInSortedLists(false);
  }
}

void CStateManager::ClearGraveyard() {
  rstl::list< rstl::reserved_vector< TUniqueId, 32 > >::iterator gyIt = mGraveyard.begin();
  for (; gyIt != mGraveyard.end(); ++gyIt) {
    rstl::reserved_vector< TUniqueId, 32 >& vec = *gyIt;
    rstl::reserved_vector< TUniqueId, 32 >::iterator idIt = vec.begin();
    for (; idIt != vec.end(); ++idIt) {
      TUniqueId id = *idIt;
      CEntity* ent = mObjectLists[0]->GetValidObjectById(id);
      RemoveObject(id);
      if (ent != nullptr) {
        delete ent;
      }
    }
  }

  mGraveyard.clear();
}

void CStateManager::SetCurrentAreaId(TAreaId aid) {
  if (mNextAreaId != aid) {
    mPrevAreaId = mNextAreaId;
    UpdateRoomAcoustics(aid);
    mNextAreaId = aid;
  }

  const TAreaId& currentArea = aid;
  if (currentArea != kInvalidAreaId) {
    if (!mMapWorldInfo->IsAreaVisited(currentArea)) {
      mMapWorldInfo->SetAreaVisited(currentArea, true);
      CMapWorldInfo* mapWorldInfo = mMapWorldInfo.GetPtr();
      CWorld* world = mWorld.get();
      CMapWorld* mapWorld = world->GetMapWorld();
      mapWorld->RecalculateWorldSphere(*mapWorldInfo, *world);
    }
  }
}

void CStateManager::UpdateRoomAcoustics(TAreaId aid) {
  rstl::reserved_vector< CScriptRoomAcoustics*, 10 > areaAcoustics;
  CObjectList* allList = mObjectLists[kOL_All].get();

  for (int i = allList->GetFirstObjectIndex(); i != -1 && areaAcoustics.size() < 10;
       i = allList->GetNextObjectIndex(i)) {
    CEntity* ent = (*allList)[i];
    if (CScriptRoomAcoustics* acoustics = TCastToPtr< CScriptRoomAcoustics >(ent)) {
      if (acoustics->GetCurrentAreaId() == aid && acoustics->GetActive()) {
        areaAcoustics.push_back(acoustics);
      }
    }
  }

  if (areaAcoustics.size() > 0) {
    const int acousticsIdx = static_cast< int >(
        0.99f * (mRandom->Float() * static_cast< float >(areaAcoustics.size())));
    areaAcoustics[acousticsIdx]->EnableAuxCallbacks();
  } else {
    CScriptRoomAcoustics::DisableAuxCallbacks();
  }
}

void CStateManager::RemoveObject(TUniqueId id) {
  CEntity* ent = mObjectLists[0]->GetValidObjectById(id);
  if (ent != nullptr) {
    TEditorId editorId = ent->GetEditorId();
    if (editorId != kInvalidEditorId) {
      const rstl::pair< rstl::multimap< TEditorId, TUniqueId >::iterator,
                        rstl::multimap< TEditorId, TUniqueId >::iterator >
          range = mScriptIdMap.equal_range(editorId);
      rstl::multimap< TEditorId, TUniqueId >::iterator it = range.first;
      while (it != range.second) {
        if (it->second == id) {
          it = mScriptIdMap.erase(it);
        } else {
          ++it;
        }
      }
    }

    TAreaId areaId = ent->mAreaId;
    if (areaId != kInvalidAreaId) {
      CGameArea* area = mWorld->Area(areaId);
      if (area->IsPostConstructed()) {
        area->ObjectList()->RemoveObject(id);
      }
    }

    if (CActor* actor = TCastToPtr< CActor >(ent)) {
      mSortedListManager->Remove(actor);
    }
  }

  for (int i = 0; i < mObjectLists.size(); ++i) {
    mObjectLists[i]->RemoveObject(id);
  }
}

void CStateManager::CreateStandardGameObjects() {
  const float stepUp = gpTweakPlayer->GetStepUpHeight();
  const float stepDown = gpTweakPlayer->GetStepDownHeight();
  const float playerHeight = gpTweakPlayer->GetPlayerHeight();
  const float xyHalfExtent = gpTweakPlayer->GetPlayerXYHalfExtent();
  const float ballRadius = gpTweakPlayer->GetPlayerBallHalfExtent();
  const CAABox playerBounds(CVector3f(-xyHalfExtent, -xyHalfExtent, 0.f),
                            CVector3f(xyHalfExtent, xyHalfExtent, playerHeight));

  const TUniqueId uid = AllocateUniqueId();

  const CVector3f pos = CVector3f::Zero();
  const CRelAngle angle = CRelAngle::FromDegrees(129.6f);
  CMatrix3f mtx = CQuaternion::AxisAngle(CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes), angle)
                      .BuildTransform();
  CTransform4f xf =
      CTransform4f::FromColumns(mtx.GetColumn(kDX), mtx.GetColumn(kDY), mtx.GetColumn(kDZ), pos);

  mPlayer = rs_new CPlayer(uid, xf, playerBounds, gpTweakPlayerRes->GetBallTransitionANCSId(),
                               CVector3f(1.65f, 1.65f, 1.65f), 200.f, stepUp, stepDown, ballRadius,
                               CMaterialList(kMT_Player, kMT_Solid, kMT_GroundCollider));
  AddObject(*mPlayer);
  mCameraManager->CreateCameras(*this);
}

void CStateManager::InitializeState(unsigned int mlvlId, TAreaId aid, unsigned int mreaId) {
  CRandom16* const oldRandom = mRandom;
  mRandom = &mRandomValue;
  const bool hadRandom = oldRandom != nullptr;

  if (mInitPhase == kIP_LoadWorld) {
    CreateStandardGameObjects();
    CWorld* world = rs_new CWorld(*gpSimplePool, *gpResourceFactory, mlvlId);
    mWorld = world;
    mInitPhase = kIP_LoadFirstArea;
  }

  if (mInitPhase == kIP_LoadFirstArea) {
    if (!mShadowTex.TryCache()) {
      return;
    }

    if (!mWorld->CheckWorldComplete(this, aid, mreaId)) {
      return;
    }

    mNextAreaId = mWorld->GetCurrentAreaId();
    CGameArea* area = mWorld->Area(GetNextAreaId());
    if (mWorld->ScheduleAreaToLoad(area, *this)) {
      area->StartStreamIn(*this);
      return;
    }

    mInitPhase = kIP_Done;
  }

  SetCurrentAreaId(mNextAreaId);
  gpGameState->CurrentWorldState().SetAreaId(mNextAreaId);
  mWorld->TravelToArea(mNextAreaId, *this, CWorld::kATT_SkipAdjacent);
  UpdateRoomAcoustics(mNextAreaId);

  CObjectList* allList = mObjectLists[kOL_All].get();
  for (int i = allList->GetFirstObjectIndex(); i != -1; i = allList->GetNextObjectIndex(i)) {
    DeliverScriptMsg((*allList)[i], kInvalidUniqueId, kSM_WorldInitialized);
  }

  allList = mObjectLists[kOL_All].get();
  for (int i = allList->GetFirstObjectIndex(); i != -1; i = allList->GetNextObjectIndex(i)) {
    CScriptSpawnPoint* const spawnPoint = TCastToPtr< CScriptSpawnPoint >((*allList)[i]);
    if (spawnPoint != nullptr && spawnPoint->GetActive() && spawnPoint->FirstSpawn()) {
      const CVector3f pos = spawnPoint->GetTransform().GetTranslation();
      CVector3f lookDir = spawnPoint->GetTransform().GetForward();
      lookDir.SetZ(0.f);
      if (lookDir.CanBeNormalized()) {
        mPlayer->Teleport(CTransform4f::LookAt(pos, pos + lookDir, CVector3f::Up()), *this,
                              true);
      }

      if (gpGameState->GetInitPowerupsAtFirstSpawn()) {
        gpGameState->SetDeferPowerupInit(false);

        for (int i = CPlayerState::kIT_PowerBeam; i < CPlayerState::kIT_Max; ++i) {
          const CPlayerState::EItemType itemType = static_cast< CPlayerState::EItemType >(i);
          if (static_cast< int >(GetPlayerState()->GetPowerUp(itemType)) <
              spawnPoint->GetPowerup(itemType)) {
            GetPlayerState()->InitializePowerUp(itemType,
                                                spawnPoint->GetPowerup(itemType) -
                                                    GetPlayerState()->GetPowerUp(itemType));
          }

          if (GetPlayerState()->GetItemAmount(itemType) < spawnPoint->GetPowerup(itemType)) {
            GetPlayerState()->IncrPickUp(itemType, spawnPoint->GetPowerup(itemType) -
                                                       GetPlayerState()->GetItemAmount(itemType));
          }
        }
      }

      break;
    }
  }

  mPlayer->AsyncLoadSuit(*this);
  mCameraManager->ResetCameras(*this);

  mRandom = hadRandom ? &mRandomValue : nullptr;

  mEnvFxManager->AsyncLoadResources(*this);
}

void CStateManager::FrameBegin(unsigned int frame) {
  mInputFrameIdx = frame;
  CTexture::sCurrentFrameCount = mInputFrameIdx;
  CGraphicsPalette::sCurrentFrameCount = mInputFrameIdx;
  SwapOutTexturesToARAM(2, 0x180000);
}

void CStateManager::SwapOutTexturesToARAM(int, unsigned int) {}

const bool CStateManager::MemoryAllocatorAllocationFailedCallback(const void* obj, unsigned int) {
  return static_cast< CStateManager* >(const_cast< void* >(obj))->SwapOutAllPossibleMemory();
}

bool CStateManager::SwapOutAllPossibleMemory() {
  CFrameDelayedKiller::StallAndFlushAllAllocations();
#if !defined(TARGET_PC)
  CARAMManager::WaitForAllDMAsToComplete();
  CARAMToken::UpdateAllDMAs();
#endif
  return true;
}

void CStateManager::UpdateGameState() {}

// Original identity is unknown; retail calls this empty hook once per update.
extern "C" void nullsub_34(CStateManager*) {}

void CStateManager::MovePlatforms(float dt) {
  CObjectList* platformAndDoorList = mObjectLists[kOL_PlatformAndDoor].get();
  for (int i = platformAndDoorList->GetFirstObjectIndex(); i != -1;
       i = platformAndDoorList->GetNextObjectIndex(i)) {
    CPhysicsActor* actor = static_cast< CPhysicsActor* >((*platformAndDoorList)[i]);
    if (CPlatformAndDoorList::IsPlatform(actor) && actor != nullptr && actor->GetActive() &&
        actor->GetMass() != 0.f) {
      CGameCollision::Move(*this, *actor, dt, 0);
    }
  }
}

void CStateManager::MoveDoors(float dt) {
  CObjectList* physActorList = mObjectLists[kOL_PhysicsActor].get();
  for (int i = physActorList->GetFirstObjectIndex(); i != -1;
       i = physActorList->GetNextObjectIndex(i)) {
    CPhysicsActor* actor = static_cast< CPhysicsActor* >((*physActorList)[i]);
    if (actor != nullptr && actor->GetActive() && actor->GetMass() != 0.f) {
      CPatterned* patterned = TCastToPtr< CPatterned >(actor);
      if (patterned != nullptr) {
        bool doThink = mCinematicPause == false;
        if (doThink && patterned->GetCurrentAreaId() != kInvalidAreaId) {
          const TAreaId areaId = patterned->mAreaId;
          const CGameArea& area = mWorld->GetAreaAlways(areaId);
          float occTime;
          if (area.IsPostConstructed()) {
            occTime = area.GetPostConstructed()->mOccludedTime;
          } else {
            occTime = 0.f;
          }
          if (occTime > 5.f) {
            doThink = false;
          }
        }

        if (!doThink) {
          SendScriptMsgAlways(patterned->GetUniqueId(), kInvalidUniqueId, kSM_SuspendedMove);
          continue;
        }
      }

      if (actor != mPlayer && !CPlatformAndDoorList::IsPlatform(actor)) {
        CGameCollision::Move(*this, *actor, dt, 0);
      }
    }
  }
}

void CStateManager::CrossTouchActors() {
  CObjectList* actorList = mObjectLists[kOL_Actor].get();
  bool visits[kMaxEntities];
  memset(visits, 0, sizeof(visits));

  for (int i = actorList->GetFirstObjectIndex(); i != -1; i = actorList->GetNextObjectIndex(i)) {
    CActor* actor = static_cast< CActor* >((*actorList)[i]);
    if (actor != nullptr && actor->GetActive() && actor->GetCallTouch()) {
      const rstl::optional_object< CAABox > touchBounds = actor->GetTouchBounds();
      if (!touchBounds) {
        continue;
      }
      TEntityList nearList;
      const CMaterialFilter filter = actor->GetMaterialList().HasMaterial(kMT_Trigger)
                                         ? CMaterialFilter::MakeExclude(CMaterialList(kMT_Trigger))
                                         : CMaterialFilter::skPassEverything;
      BuildNearList(nearList, *touchBounds, filter, actor);

      for (const TUniqueId* uid = nearList.begin(); uid != nearList.end(); ++uid) {
        CActor* actor2 = static_cast< CActor* >(ObjectById(*uid));
        if (actor2 != nullptr) {
          const rstl::optional_object< CAABox > touchBounds2 = actor2->GetTouchBounds();
          if (!actor2->GetActive() || !touchBounds2) {
            continue;
          }
          if (!visits[actor2->GetUniqueId().Value()]) {
            if (touchBounds->DoBoundsOverlap(*touchBounds2)) {
              actor->Touch(*actor2, *this);
              actor2->Touch(*actor, *this);
            }

            visits[actor->GetUniqueId().Value()] = true;
          }
        }
      }
    }
  }
}

void CStateManager::Think(float dt) {
  const float deathTime = mPlayer->GetDeathTime();
  if (deathTime > 0.f) {
    mPlayer->DoThink(dt, *this);
    return;
  }

  CObjectList* const allList = mObjectLists[kOL_All].get();
  if (mGameState == kGS_SoftPaused) {
    for (int i = allList->GetFirstObjectIndex(); i != -1; i = allList->GetNextObjectIndex(i)) {
      CScriptEffect* effect = TCastToPtr< CScriptEffect >((*allList)[i]);
      if (effect != nullptr) {
        effect->Think(dt, *this);
      }
    }
  } else {
    CObjectList* const camList = mObjectLists[kOL_GameCamera].get();
    for (int i = allList->GetFirstObjectIndex(); i != -1; i = allList->GetNextObjectIndex(i)) {
      CEntity* ent = (*allList)[i];
      if (ent != nullptr) {
        CPatterned* patterned = TCastToPtr< CPatterned >(ent);
        if (patterned != nullptr) {
          bool doThink = mCinematicPause == false;
          if (doThink && patterned->GetCurrentAreaId() != kInvalidAreaId) {
            const TAreaId areaId = patterned->mAreaId;
            const CGameArea& area = mWorld->GetAreaAlways(areaId);
            float occTime;
            if (area.IsPostConstructed()) {
              occTime = area.GetPostConstructed()->mOccludedTime;
            } else {
              occTime = 0.f;
            }
            if (occTime > 5.f) {
              doThink = false;
            }
          }

          if (!doThink) {
            continue;
          }
        }

        if (camList->GetObjectById(ent->GetUniqueId()) == nullptr) {
          ent->Think(dt, *this);
        }
      }
    }
  }
}

void CStateManager::PreThinkObjects(float dt) {
  const float deathTime = mPlayer->GetDeathTime();
  if (deathTime > 0.f) {
    mPlayer->DoPreThink(dt, *this);
    return;
  }

  CObjectList* const allList = mObjectLists[kOL_All].get();
  if (mGameState == kGS_SoftPaused) {
    for (int i = allList->GetFirstObjectIndex(); i != -1; i = allList->GetNextObjectIndex(i)) {
      CScriptEffect* effect = TCastToPtr< CScriptEffect >((*allList)[i]);
      if (effect != nullptr) {
        effect->PreThink(dt, *this);
      }
    }
  } else {
    CObjectList* const camList = mObjectLists[kOL_GameCamera].get();
    for (int i = allList->GetFirstObjectIndex(); i != -1; i = allList->GetNextObjectIndex(i)) {
      CEntity* ent = (*allList)[i];
      if (ent != nullptr && camList->GetObjectById(ent->GetUniqueId()) == nullptr) {
        ent->PreThink(dt, *this);
      }
    }
  }
}

void CStateManager::PostUpdatePlayer(float dt) { mPlayer->DoPostCameraStuff(dt, *this); }

void CStateManager::Update(float dt) {
  CElementGen::SetGlobalSeed(static_cast< ushort >(mUpdateFrameIdx));
  CParticleElectric::SetGlobalSeed(static_cast< ushort >(mUpdateFrameIdx));
  CDecal::SetGlobalSeed(static_cast< ushort >(mUpdateFrameIdx));
  CProjectileWeapon::SetGlobalSeed(mUpdateFrameIdx);

  mCurTimeMod900 += dt;
  if (mCurTimeMod900 > 900.f) {
    mCurTimeMod900 -= 900.f;
  }

  mPauseHudMessage = kInvalidAssetId;

  CScriptEffect::ResetParticleCounts();
  UpdateThermalVisor();
  nullsub_34(this);
  UpdateGameState();

  const float deathTime = mPlayer->GetDeathTime();
  const bool isDead = deathTime > 0.f;

  if (mGameState == kGS_Running) {
    if (TCastToPtr< CCinematicCamera >(
            const_cast< CGameCamera& >(mCameraManager->GetCurrentCamera(*this))) == nullptr) {
      gpGameState->SetTotalPlayTime(dt + gpGameState->GetTotalPlayTime());
      UpdateHintState(dt);
    }

    CCameraFilterPass* filt = mCamFilterPasses.data();
    CCameraBlurPass* blur = mCamBlurPasses.data();
    for (int i = 0; i < kCFS_Max; ++i) {
      filt->Update(dt);
      blur->Update(dt);
      ++filt;
      ++blur;
    }
  }

  if (mGameState != kGS_Paused) {
    PreThinkObjects(dt);
    mFluidPlaneManager->Update(dt);
  }

  if (mGameState == kGS_Running) {
    if (!isDead) {
      CDecalManager::Update(dt, *this);
    }

    UpdateSortedLists();

    if (!isDead) {
      MovePlatforms(dt);
      MoveDoors(dt);
    }

    ProcessPlayerInput();

    if (mGameState != kGS_SoftPaused) {
      CGameCollision::Move(*this, *mPlayer, dt, nullptr);
    }

    UpdateSortedLists();

    if (!isDead) {
      CrossTouchActors();
    }
  } else {
    ProcessPlayerInput();
  }

  if (!isDead && mGameState == kGS_Running) {
    mActorModelParticles->Update(dt, *this);
  }

  if (mGameState == kGS_Running || mGameState == kGS_SoftPaused) {
    Think(dt);
  }

  if (mGameState != kGS_SoftPaused) {
    mCameraManager->Update(dt, *this);
  }

  while (mLastRelay != kInvalidUniqueId) {
    CEntity* ent = ObjectById(mLastRelay);
    if (ent == nullptr) {
      mLastRelay = kInvalidUniqueId;
      break;
    }

    ent->Think(dt, *this);
  }

  if (mGameState != kGS_Paused) {
    PostUpdatePlayer(dt);
  }

  if (xf84_ == mHudMessageFrameCount) {
    ShowPausedHUDMemo(xf88_, xf8c_);
    --xf84_;
    xf88_ = kInvalidAssetId;
  }

  if (!isDead && mGameState == kGS_Running && !mCameraManager->IsInCinematicCamera()) {
    UpdateEscapeSequenceTimer(dt);
  }

  mWorld->Update(dt);
  mRumbleManager->Update(dt);

  if (!isDead) {
    mEnvFxManager->Update(dt, *this);
  }

  UpdateAreaSounds();

  mReadyToRender = true;

  if (mInMapScreen) {
    CHintOptions& hintOptions = gpGameState->HintOptions();
    CHintOptions::SHintState* hint =
        const_cast< CHintOptions::SHintState* >(hintOptions.GetCurrentDisplayedHint());
    if (hint != nullptr && hint->CanContinue()) {
      hintOptions.DismissDisplayedHint();
    }
    mInMapScreen = false;
  }

  gpGameState->CurrentWorldState().SetAreaId(mNextAreaId);

  mWorld->TravelToArea(mNextAreaId, *this, CWorld::kATT_LoadAdjacent);

  ClearGraveyard();
  ++mUpdateFrameIdx;
}

void CStateManager::ProcessInput(const CFinalInput& input) {
  static CFinalInput skDefaultInput;

  if (input.ControllerNumber() == 0) {
    const CGameCamera& cam = mCameraManager->GetCurrentCamera(*this);
    bool disableInput = cam.DisablesInput();

    if (mPlayer->GetDisableInput()) {
      disableInput = true;
    }

    if (disableInput) {
      mFinalInput = skDefaultInput;
      mFinalInput.SetTime(input.Time());
    } else {
      mFinalInput = input;
    }
  }

  mCameraManager->ProcessInput(input, *this);
}

void CStateManager::ProcessPlayerInput() {
  if (mPlayer != nullptr) {
    mPlayer->ProcessInput(mFinalInput, *this);
  }
}

void CStateManager::FrameEnd() {
  CModel::FrameDone();
  gpSimplePool->Flush();
}

void CStateManager::UpdateAreaSounds() {
  rstl::reserved_vector< int, 10 > areaIds;
  areaIds.clear();
  for (CGameArea::CConstChainIterator areaIt = mWorld->GetChainHead(CWorld::kC_Alive);
       areaIt != CWorld::GetAliveAreasEnd(); ++areaIt) {
    CGameArea::EOcclusionState occState;
    if (areaIt->IsPostConstructed()) {
      occState = areaIt->GetPostConstructed()->mOcclusionState;
    } else {
      occState = CGameArea::kOS_Occluded;
    }

    if (occState == CGameArea::kOS_Visible) {
      areaIds.push_back(areaIt->GetId().Value());
    }
  }

  CSfxManager::SetActiveAreas(areaIds);
}

void CStateManager::ApplyDamage(const TUniqueId damagerId, const TUniqueId damageeId,
                                const TUniqueId radiusSender, const CDamageInfo& info,
                                const CMaterialFilter& filter, const CVector3f& knockbackVec) {
  const CEntity* damagerEnt = GetObjectById(damagerId);
  CEntity* damageeEnt = ObjectById(damageeId);
  const CActor* const damager = TCastToConstPtr< CActor >(damagerEnt);
  CActor* const damagee = TCastToPtr< CActor >(damageeEnt);
  const bool isPlayer = TCastToConstPtr< CPlayer >(damageeEnt) != nullptr;

  if (damagee != nullptr) {
    CHealthInfo* hInfo = damagee->HealthInfo(*this);
    if (hInfo != nullptr) {
      CVector3f position(0.f, 0.f, 0.f);
      CVector3f direction(1.f, 0.f, 0.f);
      const bool alive = hInfo->GetHP() > 0.f;

      if (damager != nullptr) {
        position = damager->GetTranslation();
        direction = damager->GetTransform().GetForward();
      }

      const bool useWeaponDir = damager != nullptr || isPlayer;
      const CDamageVulnerability* dVuln =
          useWeaponDir ? damagee->GetDamageVulnerability(position, direction, info)
                       : damagee->GetDamageVulnerability();

      if (info.GetWeaponMode().GetType() == kWT_None ||
          dVuln->WeaponHurts(info.GetWeaponMode(), CDamageVulnerability::kRD_No)) {
        const float localDamage = info.GetDamage(*dVuln);
        if (localDamage > 0.f) {
          ApplyLocalDamage(position, direction, *damagee, localDamage, info.GetWeaponMode());
        }

        damagee->SendScriptMsgs(kSS_Damage, *this, kSM_None);
        DeliverScriptMsg(damagee, damagerId, kSM_Damage);
      } else {
        damagee->SendScriptMsgs(kSS_InvulnDamage, *this, kSM_None);
        DeliverScriptMsg(damagee, damagerId, kSM_InvulnDamage);
      }

      if (alive && damager != nullptr && info.GetKnockBackPower() > 0.f) {
        CVector3f defaultDir = damagee->GetTranslation() - damager->GetTranslation();
        const CVector3f& useDir = knockbackVec.IsNonZero() ? knockbackVec : defaultDir;
        CVector3f knockDir(useDir.GetX(), useDir.GetY(), 0.0001f);
        ApplyKnockBack(*damagee, info, *dVuln, knockDir.AsNormalized(), 0.f);
      }
    }

    if (damager != nullptr && info.GetRadius() > 0.f) {
      ProcessRadiusDamage(*damager, *damagee, radiusSender, info, filter);
    }

    CWallCrawlerSwarm* swarm = TCastToPtr< CWallCrawlerSwarm >(damageeEnt);
    if (swarm != nullptr && damager != nullptr) {
      swarm->ApplyRadiusDamage(damager->GetTranslation(), info, *this);
    }
  }
}

bool CStateManager::ApplyLocalDamage(const CVector3f& pos, const CVector3f& dir, CActor& damagee,
                                     float damage, const CWeaponMode& weaponMode) {
  CHealthInfo* hInfo = damagee.HealthInfo(*this);
  if (hInfo == nullptr || damage < 0.f) {
    return false;
  }

  const float oldHp = hInfo->GetHP();
  if (oldHp <= 0.f) {
    return true;
  }

  float useDamage = damage;
  CPlayer* player = TCastToPtr< CPlayer >(damagee);
  CAi* ai = static_cast< CPatterned* >(TCastToPtr< CPatterned >(damagee));
  if (ai == nullptr) {
    ai = static_cast< CDestroyableRock* >(TCastToPtr< CDestroyableRock >(damagee));
  }

  if (player != nullptr) {
    if (mCameraManager->IsInCinematicCamera() ||
        (weaponMode.GetType() == kWT_Phazon &&
         mPlayerState->HasPowerUp(CPlayerState::kIT_PhazonSuit))) {
      return false;
    }

    if (gpGameState->GetHardMode()) {
      useDamage *= gpGameState->GetHardModeDamageMultiplier();
    }

    float damageReduction = 0.f;
    if (mPlayerState->HasPowerUp(CPlayerState::kIT_VariaSuit)) {
      damageReduction = gpTweakPlayer->GetVariaDamageReduction();
    }

    if (mPlayerState->HasPowerUp(CPlayerState::kIT_GravitySuit)) {
      const float gravityReduction = gpTweakPlayer->GetGravityDamageReduction();
      damageReduction = CMath::Max< float >(damageReduction, gravityReduction);
    }

    if (mPlayerState->HasPowerUp(CPlayerState::kIT_PhazonSuit)) {
      const float phazonReduction = gpTweakPlayer->GetPhazonDamageReduction();
      damageReduction = CMath::Max< float >(damageReduction, phazonReduction);
    }

    useDamage = -(damageReduction * useDamage - useDamage);
  }

  const float newHp = oldHp - useDamage;
  hInfo->SetHP(newHp);
  const bool significant = !(fabs(newHp - oldHp) < 0.00001f);

  if (player != nullptr) {
    player->TakeDamage(significant, pos, useDamage, weaponMode.GetType(), *this);
    if (newHp <= 0.f) {
      mPlayerState->SetPlayerAlive(false);
    }
  } else if (ai != nullptr) {
    if (significant) {
      ai->TakeDamage(dir, useDamage);
    }

    if (newHp <= 0.f) {
      ai->Death(*this, dir, kSS_DeathRattle);
    }
  }

  return significant;
}

void CStateManager::TestBombHittingWater(const CActor& damager, const CVector3f& pos,
                                         CActor& damagee) {
  int idx = 0;
  if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(damager)) {
    const int attribField = weapon->GetAttribField();
    if ((attribField & (CWeapon::kPA_Bombs | CWeapon::kPA_PowerBombs)) != 0) {
      if ((attribField & CWeapon::kPA_PowerBombs) != 0) {
        idx = 1;
      }
      if (CScriptWater* const water = TCastToPtr< CScriptWater >(damagee)) {
        CVector3f hitPos(pos.GetX(), pos.GetY(), water->GetTriggerBoundsWR().GetMaxPoint().GetZ());
        const float depth = -CPlane(water->GetTriggerBoundsWR().GetMaxPoint().GetZ(),
                                    CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes))
                                 .GetHeight(pos);

        if (depth <= skBombUnderwaterRanges[idx] && depth > 0.f) {
          const float rippleFactor = 1.f - depth / skBombUnderwaterRanges[idx];
          if (mFluidPlaneManager->GetLastRippleDeltaTime(damager.GetUniqueId()) >= 0.15f) {
            const float& bombMag = skBombUnderwaterMags[idx];
            const float mag = 0.6f * bombMag +
                              0.4f * bombMag * CMath::FastSinR((2.f * M_PIF * rippleFactor) / 4.f);
            water->FluidPlane().AddRipple(mag, damager.GetUniqueId(), hitPos, *water, *this);
          }

          if (idx == 0) {
            mFluidPlaneManager->CreateSplash(damager.GetUniqueId(), *this, *water, hitPos,
                                                 rippleFactor, true);
          }
        } else if (depth > -skBombAboveWaterRanges[idx] && depth < 0.f) {
          const CVector3f down(0.f, 0.f, -1.f);
          const CRayCastResult result =
              RayStaticIntersection(pos, down, -depth, CMaterialFilter::GetPassEverything());

          if (!result.GetValid() &&
              mFluidPlaneManager->GetLastRippleDeltaTime(damager.GetUniqueId()) >= 0.15f) {
            const float& bombMag = skBombAboveWaterMags[idx];
            const float mag =
                0.6f * bombMag +
                0.4f * bombMag *
                    CMath::FastSinR((2.f * M_PIF * (-depth / skBombAboveWaterRanges[idx])) / 4.f);
            water->FluidPlane().AddRipple(mag, damager.GetUniqueId(), hitPos, *water, *this);
          }
        }
      }
    }
  }
}

const bool CStateManager::MultiRayCollideWorld(const CMRay& ray,
                                               const CMaterialFilter& filter) const {
  CVector3f crossed2 =
      CVector3f(ray.GetDirection().GetY(), -ray.GetDirection().GetZ(), ray.GetDirection().GetX());
  CVector3f crossed = CVector3f::Cross(crossed2, ray.GetDirection()).AsNormalized();
  crossed2 = 0.35355338f * CVector3f::Cross(ray.GetDirection(), crossed);
  crossed *= 0.35355338f;

  bool result = false;
  for (int i = 0; i < 4; ++i) {
    const CVector3f start =
        ray.GetStart() + ((i & 1) ? crossed : -crossed) + ((i & 2) ? -crossed2 : crossed2);
    result = CGameCollision::RayStaticIntersectionBool(*this, start, ray.GetDirection(),
                                                       ray.GetLength(), filter);
    if (result) {
      break;
    }
  }
  return result;
}

const bool CStateManager::TestRayDamage(const CVector3f& pos, const CActor& damagee,
                                        const TEntityList& nearList) const {
  if (damagee.GetHealthInfo(*this) == nullptr) {
    return false;
  }

  static const CMaterialList incList = CMaterialList(kMT_Solid);
  static const CMaterialList exList =
      CMaterialList(kMT_ProjectilePassthrough, kMT_Player, kMT_Occluder, kMT_Character);
  static const CMaterialFilter filter =
      CMaterialFilter(incList, exList, CMaterialFilter::kFT_IncludeExclude);

  const rstl::optional_object< CAABox > bounds = damagee.GetTouchBounds();
  if (!bounds) {
    return false;
  }

  const CVector3f center = bounds->GetCenterPoint();
  CVector3f dir = center - pos;
  if (dir.CanBeNormalized()) {
    const float mag = dir.Magnitude();
    dir *= 1.f / mag;
    if (RayCollideWorld(pos, center, nearList, filter, &damagee)) {
      return true;
    }

    const CMRay ray = CMRay(pos, dir, mag);
    if (!MultiRayCollideWorld(ray, filter)) {
      return false;
    }

    float depth;
    CVector3f normal = CVector3f::Zero();
    const int count = CollisionUtil::RayAABoxIntersection(ray, *bounds, normal, depth);
    if (count == 0) {
      return true;
    }
    if (count == 1) {
      return true;
    }
    return CGameCollision::RayDynamicIntersectionBool(*this, pos, dir, filter, nearList, &damagee,
                                                      depth * mag);
  }
  return true;
}

void CStateManager::ApplyRadiusDamage(const CActor& damager, const CVector3f& pos, CActor& damagee,
                                      const CDamageInfo& info) {
  CVector3f delta = damagee.GetTranslation() - pos;
  if (!(delta.MagSquared() < info.GetRadius() * info.GetRadius())) {
    if (!damagee.GetTouchBounds()) {
      return;
    }

    if (!CCollidableSphere::Sphere_AABox_Bool(CSphere(pos, info.GetRadius()),
                                              *damagee.GetTouchBounds())) {
      return;
    }
  }

  float rad = info.GetRadius();
  rad = rad > gkEpsilon ? delta.Magnitude() / rad : 0.f;

  if (rad > 0.f) {
    delta.Normalize();
  }

  const CHealthInfo* hInfo = damagee.HealthInfo(*this);
  const bool alive = hInfo != nullptr && hInfo->GetHP() > 0.f;

  const CDamageVulnerability* vuln = rad > 0.f ? damagee.GetDamageVulnerability(pos, delta, info)
                                               : damagee.GetDamageVulnerability();

  if (vuln->WeaponHurts(info.GetWeaponMode(), CDamageVulnerability::kRD_Yes)) {
    const float localDamage = info.GetRadiusDamage(*vuln);
    if (localDamage > 0.f) {
      ApplyLocalDamage(pos, delta, damagee, localDamage, info.GetWeaponMode());
    }

    damagee.SendScriptMsgs(kSS_Damage, *this, kSM_None);
    DeliverScriptMsg(&damagee, damager.GetUniqueId(), kSM_Damage);
  } else {
    damagee.SendScriptMsgs(kSS_InvulnDamage, *this, kSM_None);
    DeliverScriptMsg(&damagee, damager.GetUniqueId(), kSM_InvulnDamage);
  }

  if (alive && info.GetKnockBackPower() > 0.f) {
    const CVector3f knockbackDelta = damagee.GetTranslation() - damager.GetTranslation();
    CVector3f knockbackDir(knockbackDelta.GetX(), knockbackDelta.GetY(), 0.0001f);
    ApplyKnockBack(damagee, info, *vuln, knockbackDir.AsNormalized(), rad);
  }
}

void CStateManager::ProcessRadiusDamage(const CActor& damager, CActor& damagee,
                                        TUniqueId radiusSender, const CDamageInfo& info,
                                        const CMaterialFilter& filter) {
  CMaterialFilter localFilter = filter;
  const TUniqueId damagerId = damager.GetUniqueId();
  const TUniqueId damageeId = damagee.GetUniqueId();
  const float radius = info.GetRadius();
  const CVector3f pos(damager.GetTranslation());
  const float negativeRadius = -radius;
  const CAABox aabb(pos + CVector3f(negativeRadius, negativeRadius, negativeRadius),
                    pos + CVector3f(radius, radius, radius));

  TEntityList nearList;
  BuildNearList(nearList, aabb, localFilter, nullptr);

  for (TEntityList::iterator it = nearList.begin(); it != nearList.end(); ++it) {
    CActor* actor = static_cast< CActor* >(ObjectById(*it));
    if (actor != nullptr) {
      const TUniqueId actorId = actor->GetUniqueId();
      if (damagerId != actorId && radiusSender != actorId && damageeId != actorId) {
        TestBombHittingWater(damager, pos, *actor);
        if (TestRayDamage(pos, *actor, nearList)) {
          ApplyRadiusDamage(damager, pos, *actor, info);
        }
      }
    }
  }
}

void CStateManager::ApplyDamageToWorld(TUniqueId damagerId, const CActor& actor,
                                       const CVector3f& pos, const CDamageInfo& info,
                                       const CMaterialFilter& filter) {
  const CMaterialFilter useFilter = filter;
  const float radius = info.GetRadius();
  const float negativeRadius = -radius;
  const CAABox aabb(pos + CVector3f(negativeRadius, negativeRadius, negativeRadius),
                    pos + CVector3f(radius, radius, radius));

  const CWeapon* const weapon = TCastToConstPtr< CWeapon >(&actor);
  bool bomb = false;
  if (weapon != nullptr) {
    bomb = (weapon->GetAttribField() & CWeapon::kPA_Bombs) == CWeapon::kPA_Bombs ||
           (weapon->GetAttribField() & CWeapon::kPA_PowerBombs) == CWeapon::kPA_PowerBombs;
  }

  TEntityList nearList;
  BuildNearList(nearList, aabb, useFilter, &actor);

  for (TEntityList::iterator it = nearList.begin(); it != nearList.end(); ++it) {
    CActor* const act = static_cast< CActor* >(ObjectById(*it));
    CPlayer* const player = TCastToPtr< CPlayer >(act);
    CWallCrawlerSwarm* const wallSwarm = TCastToPtr< CWallCrawlerSwarm >(ObjectById(*it));
    CSnakeWeedSwarm* const snakeSwarm = TCastToPtr< CSnakeWeedSwarm >(ObjectById(*it));

    if (bomb && player != nullptr) {
      if (player->GetFrozenState()) {
        gpGameState->SystemState().IncrementFrozenBallCount();
        CSamusHud::DisplayHudMemo(rstl::wstring_l(L""), CHUDMemoParms(0.f, true, true, true));
        player->BreakFrozenState(*this);
      } else if ((weapon->GetAttribField() & CWeapon::kPA_Bombs) == CWeapon::kPA_Bombs) {
        player->BombJump(pos, *this);
      }
    } else if (act != nullptr && act->GetUniqueId() != damagerId) {
      TestBombHittingWater(actor, pos, *act);
      if (TestRayDamage(pos, *act, nearList)) {
        ApplyRadiusDamage(actor, pos, *act, info);
      }
    }

    if (wallSwarm != nullptr) {
      wallSwarm->ApplyRadiusDamage(pos, info, *this);
    }

    if (snakeSwarm != nullptr) {
      snakeSwarm->ApplyRadiusDamage(pos, info, *this);
    }
  }
}

void CStateManager::ApplyKnockBack(CActor& actor, const CDamageInfo& info,
                                   const CDamageVulnerability& vuln, const CVector3f& dir,
                                   float dampen) {
  if (vuln.GetVulnerability(info.GetWeaponMode(), CDamageVulnerability::kRD_No) == kVN_Deflect) {
    return;
  }

  CHealthInfo* hInfo = actor.HealthInfo(*this);
  if (hInfo == nullptr) {
    return;
  }

  const float dampedPower = (1.f - dampen) * info.GetKnockBackPower();
  const float resistance = hInfo->GetKnockBackResistance();

  CPlayer* player = TCastToPtr< CPlayer >(actor);
  CPatterned* patterned = TCastToPtr< CPatterned >(actor);

  if (player != nullptr) {
    KnockBackPlayer(*player, dir, dampedPower, resistance);
    return;
  }

  if (patterned == nullptr && hInfo->GetHP() <= 0.f) {
    if (dampedPower > resistance) {
      CPhysicsActor* const physActor = TCastToPtr< CPhysicsActor >(actor);
      if (physActor != nullptr) {
        const CVector3f impulse =
            dir * (1.5f * ((dampedPower - resistance) * physActor->GetMass()));
        if (!physActor->GetMaterialList().HasMaterial(kMT_Immovable) &&
            physActor->GetMaterialList().HasMaterial(kMT_Solid)) {
          physActor->ApplyImpulseWR(impulse, CAxisAngle::Identity());
        }
      }
    }
  } else if (patterned != nullptr) {
    patterned->KnockBack(dir, *this, info, dampedPower, dampen == 0.f, false);
  }
}

void CStateManager::KnockBackPlayer(CPlayer& player, const CVector3f& dir, float power,
                                    float resistance) {
  if (player.GetMaterialList().HasMaterial(kMT_Immovable)) {
    return;
  }

  float usePower = power;
  if (player.GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
    usePower *= 1000.f;
    if (player.GetSurfaceRestraint() != CPlayer::kSR_Normal &&
        player.GetOrbitState() == CPlayer::kOS_NoOrbit) {
      usePower /= 7.f;
    }
  } else {
    usePower *= 500.f;
  }

  const float minVel = player.GetMorphballTransitionState() == CPlayer::kMS_Morphed ? 35.f : 70.f;
  const float playerVel = player.GetVelocityWR().Magnitude();
  const float maxVel = rstl::max_val(minVel, playerVel);
  float angle = CVector3f::GetAngleDiff(dir, -player.GetVelocityWR()) * 0.5f;
  usePower *= 1.f - angle / M_PIF;

  player.ApplyImpulseWR(dir * usePower, CAxisAngle::Identity());
  player.UseCollisionImpulses();
  player.SetAccelerationChangeTimer(0.25f);

  const CVector3f velocity = player.GetVelocityWR();
  const float velocityMag = velocity.Magnitude();
  if (velocityMag > maxVel) {
    player.SetVelocityWR(maxVel * (velocity / velocityMag));
  }
}

void CStateManager::InformListeners(const CVector3f& pos, EListenNoiseType type) {
  CObjectList* list = mObjectLists[kOL_ListeningAi].get();
  for (int i = list->GetFirstObjectIndex(); i != -1; i = list->GetNextObjectIndex(i)) {
    CPatterned* patterned = TCastToPtr< CPatterned >((*list)[i]);
    if (patterned != nullptr) {
      if (patterned->GetActive()) {
        CGameArea* area = mWorld->Area(patterned->GetCurrentAreaId());
        CGameArea::EOcclusionState occState;
        if (area->IsPostConstructed()) {
          occState = area->GetPostConstructed()->mOcclusionState;
        } else {
          occState = CGameArea::kOS_Occluded;
        }

        if (occState != CGameArea::kOS_Occluded) {
          patterned->Listen(pos, type);
        }
      }
    }
  }
}

rstl::pair< TEditorId, TUniqueId > CStateManager::LoadScriptObject(TAreaId aid,
                                                                   EScriptObjectType type,
                                                                   unsigned int length,
                                                                   CInputStream& in) {
  uint bytesLeft = length;
  bool failed = false;
  const TEditorId eid = in.ReadLong();

  rstl::vector< SConnection > conns;
  const int connCount = in.Get< int >();
  bytesLeft -= 8;
  conns.reserve(connCount);

  for (int i = 0; i < connCount; ++i) {
    const EScriptObjectState state = static_cast< EScriptObjectState >(in.ReadLong());
    const EScriptObjectMessage msg = static_cast< EScriptObjectMessage >(in.ReadLong());
    const TEditorId target = in.ReadLong();
    bytesLeft -= 12;
    conns.push_back(SConnection(state, msg, target));
  }

  const uint propCount = in.ReadLong();
  bytesLeft -= 4;
  const uint readPos = in.GetReadPosition();

  CEntity* ent = nullptr;
  FScriptLoader loader = nullptr;
  if (type <= kST_EnergyBall && type >= kST_Actor) {
    loader = mLoaderFuncs[type];
  }

  if (loader != nullptr) {
    CEntityInfo info(aid, conns, eid);
    ent = loader(*this, in, propCount, info);
  } else {
    failed = true;
  }

  if (ent != nullptr) {
    AddObject(*ent);
  } else {
    failed = true;
  }

  bytesLeft -= in.GetReadPosition() - readPos;
  if (bytesLeft != 0) {
    while (bytesLeft-- != 0) {
      in.ReadChar();
    }
  }

  if (failed || ent == nullptr) {
    return rstl::pair< TEditorId, TUniqueId >(kInvalidEditorId, kInvalidUniqueId);
  }
  return rstl::pair< TEditorId, TUniqueId >(eid, ent->GetUniqueId());
}

rstl::pair< TEditorId, TUniqueId > CStateManager::GenerateObject(const TEditorId& eid) {
  const rstl::pair< const SScriptObjectStream*, TEditorId > build = GetBuildForScript(eid);
  const int areaId = build.second.AreaNum();
  CWorld* world = World();
  if (build.first != nullptr && world->Area(TAreaId(areaId))->IsPostConstructed()) {
    const CGameArea* area = world->GetArea(TAreaId(areaId));
    const rstl::pair< const uchar*, int > layerBuf =
        area->GetLayerScriptBuffer(TLayerId(build.second.value >> 26));
    CMemoryInStream stream(layerBuf.first + build.first->mPosition, build.first->mLength);
    return LoadScriptObject(TAreaId(areaId), build.first->mType, build.first->mLength, stream);
  }

  return rstl::pair< TEditorId, TUniqueId >(kInvalidEditorId, kInvalidUniqueId);
}

void CStateManager::LoadScriptObjects(TAreaId aid, CInputStream& in,
                                      rstl::vector< TEditorId >& persist) {
  in.ReadChar();

  const int count = in.ReadLong();
  int remaining = count;
  persist.reserve(count + persist.size());
  while (remaining--) {
    const char type = in.ReadChar();
    const uint length = in.ReadLong();
    const uint readPos = in.GetReadPosition();
    SScriptObjectStream stream(static_cast< EScriptObjectType >(static_cast< uchar >(type)),
                               readPos, length);

    const rstl::pair< TEditorId, TUniqueId > loaded = LoadScriptObject(
        aid, static_cast< EScriptObjectType >(static_cast< uchar >(type)), length, in);
    const TEditorId eid = loaded.first;
    if (eid != kInvalidEditorId) {
      const rstl::pair< const SScriptObjectStream*, TEditorId > build = GetBuildForScript(eid);
      if (build.first == NULL) {
        const rstl::pair< TEditorId, SScriptObjectStream > entry(eid, stream);
        mLoadedScriptObjects.insert(entry);
        persist.push_back(eid);
      }
    }
  }
}

void CStateManager::InitScriptObjects(const rstl::vector< TEditorId >& ids) {
  for (int i = 0; i < static_cast< int >(ids.size()); ++i) {
    if (ids[i] != kInvalidEditorId) {
      SendScriptMsgAlways(GetIdForScript(ids[i]), kInvalidUniqueId, kSM_InitializedInArea);
    }
  }

  MurderScriptInstanceNames();
}

CStateManager::TIdListResult CStateManager::GetIdListForScript(TEditorId eid) const {
  const TIdListResult result = mScriptIdMap.equal_range(eid);
  return result;
}

TUniqueId CStateManager::GetIdForScript(TEditorId eid) const {
  rstl::multimap< TEditorId, TUniqueId >::const_iterator it = mScriptIdMap.find(eid);
  if (it != mScriptIdMap.end()) {
    return it->second;
  }
  return kInvalidUniqueId;
}

TEditorId CStateManager::GetEditorIdForUniqueId(TUniqueId uid) const {
  const CEntity* ent = GetObjectById(uid);
  if (ent != nullptr) {
    return ent->GetEditorId();
  }
  return kInvalidEditorId;
}

rstl::pair< const SScriptObjectStream*, TEditorId >
CStateManager::GetBuildForScript(TEditorId eid) const {
  rstl::map< TEditorId, SScriptObjectStream >::const_iterator it =
      mLoadedScriptObjects.find(eid);
  if (it != mLoadedScriptObjects.end()) {
    return rstl::pair< const SScriptObjectStream*, TEditorId >(&it->second, it->first);
  }

  return rstl::pair< const SScriptObjectStream*, TEditorId >(0, kInvalidEditorId);
}

void CStateManager::FreeScriptObjects(TAreaId aid) {
  rstl::multimap< TEditorId, TUniqueId >::iterator scriptIt = mScriptIdMap.begin();
  while (scriptIt != mScriptIdMap.end()) {
    rstl::multimap< TEditorId, TUniqueId >::iterator cur = scriptIt;
    ++scriptIt;

    if (cur->first.AreaNum() == aid.Value()) {
      DeleteObjectRequest(cur->second);
    }
  }

  typedef rstl::map< TEditorId, SScriptObjectStream > TLoadedScriptObjMap;
  for (TLoadedScriptObjMap::iterator loadedIt = mLoadedScriptObjects.begin();
       loadedIt != mLoadedScriptObjects.end();) {
    const TLoadedScriptObjMap::iterator cur = loadedIt++;
    if (cur->first.AreaNum() == aid.Value()) {
      mLoadedScriptObjects.erase(cur);
    }
  }

  CGameArea* area = mWorld->Area(aid);
  if (area->IsPostConstructed()) {
    CGameArea::CAreaObjectList* areaObjList = area->ObjectList();

    for (int i = areaObjList->GetFirstObjectIndex(); i != -1;
         i = areaObjList->GetNextObjectIndex(i)) {
      CEntity* ent = (*areaObjList)[i];
      if (ent != nullptr && !ent->mNotInArea) {
        DeleteObjectRequest(ent->GetUniqueId());
      }
    }
  }
}

void CStateManager::SendScriptMsg(TUniqueId uid, TEditorId target, EScriptObjectMessage msg,
                                  EScriptObjectState) {
  GetObjectById(uid);
  CObjectList* allList = mObjectLists[kOL_All].get();

  TIdListResult search = GetIdListForScript(target);
  if (search.first == search.second) {
    return;
  }

  TIdList::const_iterator it = search.first;
  for (; it != search.second; ++it) {
    CEntity* ent = allList->GetObjectById(it->second);
    DeliverScriptMsg(ent, uid, msg);
  }
}

void CStateManager::RecursiveDrawTree(TUniqueId uid) const {
  CActor* actor = TCastToPtr< CActor >(const_cast< CEntity* >(GetObjectById(uid)));
  if (actor != NULL && mObjectDrawToken != actor->GetDrawToken()) {
    const TUniqueId nextNode = actor->GetDrawParent();
    if (nextNode != kInvalidUniqueId) {
      RecursiveDrawTree(nextNode);
    }
    if (mObjectDrawToken == actor->GetAddedToken()) {
      actor->Render(*this);
    }
    actor->SetDrawToken(mObjectDrawToken);
  }
}

void CStateManager::RendererDrawCallback(const void* drawable, const void* context, int type) {
  const CStateManager& mgr = *static_cast< const CStateManager* >(context);
  switch (type) {
  case 0: {
    const CActor& actor = *static_cast< const CActor* >(drawable);
    if (mgr.mObjectDrawToken == actor.GetDrawToken()) {
      break;
    }
    const TUniqueId nextNode = actor.GetDrawParent();
    if (nextNode != kInvalidUniqueId) {
      mgr.RecursiveDrawTree(nextNode);
    }
    actor.Render(mgr);
    actor.SetDrawToken(mgr.mObjectDrawToken);
    break;
  }
  case 1:
    static_cast< const CSimpleShadow* >(drawable)->Render(mgr.mShadowTex.GetObject());
    break;
  case 2:
    static_cast< const CDecal* >(drawable)->Render();
    break;
  }
}

bool CStateManager::GetVisSetForArea(const TAreaId areaA, const TAreaId areaB,
                                     CPVSVisSet& setOut) const {
  if (areaB == kInvalidAreaId) {
    return false;
  }

  CVector3f closestDockPoint = CGraphics::GetViewMatrix().GetTranslation();
  const CVector3f viewPoint = closestDockPoint;

  bool hasClosestDock = false;
  if (areaA == areaB) {
    hasClosestDock = true;
  } else {
    const CGameArea* area = mWorld->GetArea(areaB);
    if (area->IsPostConstructed()) {
      const int dockCount = area->GetDockCount();
      for (int i = 0; i < dockCount; ++i) {
        const IGameArea::Dock& dock = area->GetDock(i);
        const int connCount = dock.GetDockRefs().size();
        for (int conn = 0; conn < connCount; ++conn) {
          if (dock.GetConnectedAreaId(conn) != areaA) {
            continue;
          }

          const rstl::reserved_vector< CVector3f, 4 >& verts = dock.GetPlaneVertices();
          const CVector3f dockCenter = 0.25f * (verts[0] + verts[1] + verts[2] + verts[3]);

          if (hasClosestDock) {
            if (!((dockCenter - viewPoint).MagSquared() <
                  (closestDockPoint - viewPoint).MagSquared())) {
              continue;
            }
          }

          closestDockPoint = dockCenter;
          hasClosestDock = true;
        }
      }
    }
  }

  int setState = 0;
  if (hasClosestDock) {
    setState = 1;

    const CGameArea* area = mWorld->GetArea(areaA);
    const CPVSAreaSet* areaSet = area->GetPostConstructed()->mPvs.get();
    if (areaSet != nullptr) {
      setState = 2;

      CPVSVisOctree& visOctree = areaSet->GetVisOctree();

      const CTransform4f& invAreaXf = mWorld->GetArea(areaA)->GetInverseTransform();
      const CVector3f localPoint = invAreaXf * closestDockPoint;

      CPVSVisSet visSet = visOctree.GetVisSet(localPoint);
      if (visSet.GetState() == kVSS_NodeFound) {
        setState = 3;
        setOut = visSet;
      }
    }
  }

  return setState == 3;
}

CGameArea::CChainIterator CWorld::AliveAreasEnd() { return skGlobalNonConstEnd; }

void CStateManager::PreRender() {
  if (!mReadyToRender) {
    return;
  }

  CStopwatch timer;

  mStateManagerContainer->xf370_.clear();
  mStateManagerContainer->mRenderLast.clear();
  mProjectedShadow = nullptr;

  mWorld->PreRender();
  BuildDynamicLightListForWorld();

  const CGameCamera& curCam = mCameraManager->GetCurrentCamera(*this);
  const CTransform4f curCamXf = mCameraManager->GetCurrentCameraTransform(*this);
  CFrustumPlanes frustum(curCamXf, 0.017453292f * curCam.GetFov(), curCam.GetAspectRatio(),
                         curCam.GetNearClipDistance(), false, 100.f);

  for (CGameArea::CChainIterator areaIt = mWorld->ChainHead(CWorld::kC_Alive);
       areaIt != CWorld::AliveAreasEnd(); ++areaIt) {
    CGameArea::EOcclusionState occState;
    if (areaIt->IsPostConstructed()) {
      occState = areaIt->GetPostConstructed()->mOcclusionState;
    } else {
      occState = CGameArea::kOS_Occluded;
    }

    if (occState == CGameArea::kOS_Visible) {
      CObjectList* const areaObjList =
          static_cast< CObjectList* >(areaIt->GetPostConstructed()->mAreaObjectList.get());
      for (int i = areaObjList->GetFirstObjectIndex(); i != -1;
           i = areaObjList->GetNextObjectIndex(i)) {
        CActor* actor = TCastToPtr< CActor >((*areaObjList)[i]);
        if (actor != nullptr && actor->IsDrawEnabled()) {
          actor->CalculateRenderBounds();
          actor->PreRender(*this, frustum);
        }
      }
    }
  }

  if (!gkWorldOnlyReflection) {
    CacheReflection();
  }

  gpRender->PrepareDynamicLights(mDynamicLights);
  sPreRenderStepTime = timer.GetElapsedMicros();
}

CFrustumPlanes CStateManager::SetupViewForDraw(const CViewport& viewport) const {
  const CGameCamera& cam = mCameraManager->GetCurrentCamera(*this);
  const CTransform4f camXf = mCameraManager->GetCurrentCameraTransform(*this);
  gpRender->SetWorldViewpoint(camXf);

  const CVector3f playerPos = mPlayer->GetTranslation();
  CCubeModel::SetNewPlayerPositionAndTime(playerPos, CStopwatch::GetGlobalTimerObj());

  const float scaledWidth = mViewportScaleX * static_cast< float >(viewport.mWidth);
  const float scaledHeight = mViewportScaleY * static_cast< float >(viewport.mHeight);
  const float aspect = (mViewportScaleX * cam.GetAspectRatio()) / mViewportScaleY;
  const int width = static_cast< int >(scaledWidth);
  const int height = (static_cast< int >(scaledHeight) / 2) * 2;

  const int left = viewport.mLeft + (viewport.mWidth - width) / 2;
  const int top = viewport.mTop + (viewport.mHeight - height) / 2;

  const float tangent = CMath::SlowTangentR(CMath::Deg2Rad(0.5f * cam.GetFov()));
  const float fov = 2.f * CMath::ArcTangentR(tangent * mViewportScaleY);

  gpRender->SetViewport(left, top, width, height);
  CGraphics::SetDepthRange(0.125f, 1.f);

  const float zFar = cam.GetFarClipDistance();
#if defined(TARGET_PC)
  gpRender->SetPerspective(360.f * ((1.f / (2.f * M_PIF)) * fov), aspect,
                           cam.GetNearClipDistance(), zFar);
#else
  gpRender->SetPerspective(360.f * ((1.f / (2.f * M_PIF)) * fov), scaledWidth, scaledHeight,
                           cam.GetNearClipDistance(), zFar);
#endif

  CFrustumPlanes frustum(camXf, fov, aspect, cam.GetNearClipDistance(), false, 100.f);
  gpRender->SetClippingPlanes(frustum);
  gpRender->PrimColor(CColor::White());
  gpRender->SetModelMatrix(CTransform4f::Identity());

  mFluidPlaneManager->StartFrame(false);
  gpRender->SetDebugOption(IRenderer::kDO_PVSState, 1);

  return frustum;
}

bool CStateManager::SetupFogForDraw() const {
  switch (mPlayerState->GetActiveVisor(*this)) {
  case CPlayerState::kPV_Thermal:
    gpRender->SetWorldFog(kRFM_None, 0.f, 1.f, CColor::Black());
    return true;
  case CPlayerState::kPV_XRay:
    return false;
  default:
    return false;
  case CPlayerState::kPV_Combat:
  case CPlayerState::kPV_Scan: {
    const CGameArea::CAreaFog* fog = &mCameraManager->GetFog();
    if (fog->IsFogDisabled()) {
      return false;
    }
    fog->SetCurrent();
    return true;
  }
  }
}

void CStateManager::SetupFogForArea(const CGameArea& area) const {
  if (SetupFogForDraw()) {
    return;
  }

  if (mPlayerState->GetActiveVisor(*this) == CPlayerState::kPV_XRay) {
    const float fogDist = const_cast< CGameArea& >(area).GetXRayFogDistance();
    const CTweakGui* tweak = gpTweakGui;
    const float fogFarZ = tweak->GetXRayFogFarZ();
    const float nearZ = tweak->GetXRayFogNearZ();
    const float farZ = nearZ * (1.f - fogDist) + fogFarZ * fogDist;
    gpRender->SetWorldFog(static_cast< ERglFogMode >(tweak->GetXRayFogMode()), nearZ, farZ,
                          tweak->GetXRayFogColor());
  } else {
    area.GetPostConstructed()->mAreaFog->SetCurrent();
  }
}

void CStateManager::SetupFogForArea3XRange(const CGameArea& area) const {
  if (mPlayerState->GetActiveVisor(*this) != CPlayerState::kPV_XRay) {
    return;
  }

  const float fogDist = const_cast< CGameArea& >(area).GetXRayFogDistance();
  const CTweakGui* tweak = gpTweakGui;
  const float fogFarZ = tweak->GetXRayFogFarZ();
  const float nearZ = tweak->GetXRayFogNearZ();
  float farZ = nearZ * (1.f - fogDist) + fogFarZ * fogDist;
  farZ *= 3.f;
  gpRender->SetWorldFog(static_cast< ERglFogMode >(tweak->GetXRayFogMode()), nearZ, farZ,
                        tweak->GetXRayFogColor());
}

void CStateManager::SetupFogForArea(TAreaId area) const {
  TAreaId areaValue;
  const TAreaId* areaId = &area;
  int nextArea;
  if (area == kInvalidAreaId) {
    nextArea = mNextAreaId.Value();
    areaId = reinterpret_cast< const TAreaId* >(&nextArea);
  }

  areaValue = *areaId;
  const CGameArea* areaObj = mWorld->GetArea(areaValue);
  if (areaObj->IsPostConstructed()) {
    SetupFogForArea(*areaObj);
  }
}

void CStateManager::SetupFogForArea3XRange(TAreaId area) const {
  TAreaId areaValue;
  const TAreaId* areaId = &area;
  int nextArea;
  if (area == kInvalidAreaId) {
    nextArea = mNextAreaId.Value();
    areaId = reinterpret_cast< const TAreaId* >(&nextArea);
  }

  areaValue = *areaId;
  const CGameArea* areaObj = mWorld->GetArea(areaValue);
  if (areaObj->IsPostConstructed()) {
    SetupFogForArea3XRange(*areaObj);
  }
}

CGameArea::CConstChainIterator CWorld::GetAliveAreasEnd() { return skGlobalEnd; }

void CStateManager::DrawWorld() const {
  const CTimeProvider timeProvider(mCurTimeMod900);
  const CViewport backupViewport = CGraphics::GetViewport();
  const CFrustumPlanes frustum = SetupViewForDraw(backupViewport);
  const CTransform4f backupViewMatrix = CGraphics::GetViewMatrix();
  const TAreaId visAreaId = GetVisAreaId();
  const rstl::reserved_vector< TUniqueId, 20 >& renderLast =
      mStateManagerContainer->mRenderLast;
  const rstl::reserved_vector< TUniqueId, 20 >& renderFirst = mStateManagerContainer->xf370_;
  rstl::reserved_vector< const CGameArea*, 10 > areas;
  rstl::reserved_vector< CPVSVisSet, 10 > visibility;

  mWorld->TouchSky();
  for (CGameArea::CConstChainIterator it = mWorld->GetChainHead(CWorld::kC_Alive);
       it != CWorld::GetAliveAreasEnd() && areas.size() != 10; ++it) {
    if (it->GetOcclusionState() == CGameArea::kOS_Visible) {
      areas.push_back(&*it);
    }
  }

  rstl::sort(areas.begin(), areas.end(), area_sorter(backupViewMatrix.GetForward(), visAreaId));
  for (const CGameArea** it = areas.begin(); it != areas.end(); ++it) {
    CPVSVisSet set(kVSS_OutOfBounds);
    GetVisSetForArea((*it)->GetId(), visAreaId, set);
    visibility.push_back(set);
  }

  const CPlayerState::EPlayerVisor visor = mPlayerState->GetActiveVisor(*this);
  uint mask;
  bool thermal = visor == CPlayerState::kPV_Thermal;
  uint targetMask;
  if (thermal) {
    mThermalFlag = kTD_Cold;
    mask = 0x34;
    targetMask = 0;
  } else {
    int bit = visor == CPlayerState::kPV_XRay ? 3 : 1;
    mThermalFlag = kTD_Bypass;
    mask = 1 << bit;
    targetMask = 0;
  }

  gpRender->SetThermal(thermal, gpTweakGui->GetThermalVisorLevel(),
                       gpTweakGui->GetThermalVisorColor());
  gpRender->SetThermalColdScale(mThermColdScale2 + mThermColdScale1);
  for (int i = areas.size() - 1; i >= 0; --i) {
    const CGameArea& area = *areas[i];
    SetupFogForArea(area);
    const TAreaId id = area.GetId();
    gpRender->EnablePVS(&visibility[i], id.Value());
    gpRender->SetWorldLightFadeLevel(area.GetPostConstructed()->mWorldLightingLevel);
    gpRender->DrawUnsortedGeometry(id.Value(), mask, targetMask);
  }

  if (!SetupFogForDraw()) {
    gpRender->SetWorldFog(kRFM_None, 0.f, 1.f, CColor::Black());
  }
  mWorld->DrawSky(CTransform4f::Translate(backupViewMatrix.GetTranslation()));
  if (!areas.empty()) {
    SetupFogForArea(*areas.back());
  }

  for (const TUniqueId* it = renderFirst.begin(); it != renderFirst.end(); ++it) {
    if (const CActor* actor = static_cast< const CActor* >(GetObjectById(*it))) {
      if (!thermal || (actor->GetThermalFlags() & 1) != 0) {
        actor->Render(*this);
      }
    }
  }

  bool morphingPlayerVisible = false;
  rstl::reserved_vector< const CActor*, 1024 > thermalActors;
  for (int i = 0; i < areas.size(); ++i) {
    const CGameArea& area = *areas[i];
    const CPVSVisSet& set = visibility[i];
    const bool isVisArea = area.GetId() == visAreaId;
    SetupFogForArea(area);
    gpRender->SetWorldLightFadeLevel(area.GetPostConstructed()->mWorldLightingLevel);
    const CObjectList& objects = *area.GetPostConstructed()->mAreaObjectList;
    for (int idx = objects.GetFirstObjectIndex(); idx != -1;
         idx = objects.GetNextObjectIndex(idx)) {
      if (const CActor* actor = TCastToConstPtr< CActor >(objects[idx])) {
        if (!actor->IsDrawEnabled()) {
          continue;
        }
        const TUniqueId id = actor->GetUniqueId();
        if (!thermal && const_cast< CGameArea& >(area).LookupPVSUniqueID(id) == id) {
          if (set.GetVisible(const_cast< CGameArea& >(area).LookupPVSID(id)) == kVSS_EndOfTree) {
            continue;
          }
        }
        if (actor == mPlayer) {
          if (!thermal) {
            const CPlayer::EPlayerMorphBallState state = mPlayer->GetMorphballTransitionState();
            if (state == CPlayer::kMS_Morphed || state == CPlayer::kMS_Unmorphed) {
              mPlayer->AddToRenderer(frustum, *this);
            } else {
              morphingPlayerVisible = true;
            }
          }
        } else {
          if (!thermal || (actor->GetThermalFlags() & 1) != 0) {
            actor->AddToRenderer(frustum, *this);
          }
          if (thermal && (actor->GetThermalFlags() & 2) != 0) {
            thermalActors.push_back(actor);
          }
        }
      }
    }

    if (isVisArea && !thermal) {
      CDecalManager::AddToRenderer(frustum, *this);
      mActorModelParticles->AddStragglersToRenderer(*this);
    }
    ++mObjectDrawToken;
    mPlayer->MorphBall()->DrawBallShadow(const_cast< CStateManager& >(*this));
#if VERSION >= VERSION_GM8P_00
    for (const CProjectedShadow* shadow = mProjectedShadow; shadow != nullptr;
         shadow = shadow->GetNextShadow()) {
      shadow->Render(*this);
    }
#else
    if (mProjectedShadow != nullptr) {
      mProjectedShadow->Render(*this);
    }
#endif
    gpRender->EnablePVS(&set, area.GetId().Value());
    gpRender->DrawSortedGeometry(area.GetId().Value(), mask, targetMask);
  }

  mEnvFxManager->Render(*this);
  if (morphingPlayerVisible) {
    mPlayer->Render(*this);
  }
  gpRender->PostRenderFogs();

  if (thermal) {
    if (!renderLast.empty()) {
      CGraphics::SetDepthRange(0.015625f, 0.03125f);
      for (const TUniqueId* it = renderLast.begin(); it != renderLast.end(); ++it) {
        if (const CActor* actor = static_cast< const CActor* >(GetObjectById(*it))) {
          if ((actor->GetThermalFlags() & 1) != 0) {
            actor->Render(*this);
          }
        }
      }
      CGraphics::SetDepthRange(0.125f, 1.f);
    }
    gpRender->DoThermalBlendCold();
    mThermalFlag = kTD_Hot;
    for (const TUniqueId* it = renderFirst.begin(); it != renderFirst.end(); ++it) {
      if (const CActor* actor = static_cast< const CActor* >(GetObjectById(*it))) {
        if ((actor->GetThermalFlags() & 2) != 0) {
          actor->Render(*this);
        }
      }
    }

    for (int i = areas.size() - 1; i >= 0; --i) {
      const TAreaId id = areas[i]->GetId();
      gpRender->EnablePVS(&visibility[i], id.Value());
      gpRender->DrawUnsortedGeometry(id.Value(), mask, 0x20);
      gpRender->DrawAreaGeometry(id.Value(), mask, 0x10);
    }
    ++mObjectDrawToken;
    for (int i = 0; i < areas.size(); ++i) {
      const TAreaId id = areas[i]->GetId();
      for (const CActor** it = thermalActors.begin(); it != thermalActors.end(); ++it) {
        const CActor* actor = *it;
        if (actor->GetCurrentAreaId() == id ||
            (actor->GetCurrentAreaId() == kInvalidAreaId && id == visAreaId)) {
          actor->AddToRenderer(frustum, *this);
        }
      }
      if (i == areas.size() - 1) {
        mActorModelParticles->AddStragglersToRenderer(*this);
        CDecalManager::AddToRenderer(frustum, *this);
        if (mPlayer != nullptr) {
          mPlayer->AddToRenderer(frustum, *this);
        }
      }
      ++mObjectDrawToken;
      gpRender->EnablePVS(&visibility[i], id.Value());
      gpRender->DrawSortedGeometry(id.Value(), mask, 0x10);
    }
    gpRender->PostRenderFogs();
  }

  mFluidPlaneManager->EndFrame();
  gpRender->SetWorldFog(kRFM_None, 0.f, 1.f, CColor::Black());
  if (gkWorldOnlyReflection) {
    const_cast< CStateManager* >(this)->CacheReflection();
  }
  if (mPlayer != nullptr) {
    mPlayer->RenderGun(*this, mCameraManager->GetGlobalCameraTranslation(*this));
  }
  if (!renderLast.empty()) {
    CGraphics::SetDepthRange(0.015625f, 0.03125f);
    for (const TUniqueId* it = renderLast.begin(); it != renderLast.end(); ++it) {
      if (const CActor* actor = static_cast< const CActor* >(GetObjectById(*it))) {
        if (!thermal || (actor->GetThermalFlags() & 2) != 0) {
          actor->Render(*this);
        }
      }
    }
    CGraphics::SetDepthRange(0.125f, 1.f);
  }
  if (thermal) {
    gpRender->DoThermalBlendHot();
    gpRender->SetThermal(false, 0.f, CColor::Black());
    mThermalFlag = kTD_Bypass;
  }

  DrawDebugStuff();
  RenderCamerasAndAreaLights();
  ResetViewAfterDraw(backupViewport, backupViewMatrix);
  DrawE3DeathEffect();
  DrawAdditionalFilters();
}

void CStateManager::ResetViewAfterDraw(const CViewport& backupViewport,
                                       const CTransform4f& backupViewMatrix) const {
  gpRender->SetViewport(backupViewport.mLeft, backupViewport.mTop, backupViewport.mWidth,
                        backupViewport.mHeight);

  const CGameCamera& cam = mCameraManager->GetCurrentCamera(*this);
  CFrustumPlanes frustum(backupViewMatrix, 0.017453292f * cam.GetFov(), cam.GetAspectRatio(),
                         cam.GetNearClipDistance(), false, 100.f);
  gpRender->SetClippingPlanes(frustum);

  const CViewport& viewport = CGraphics::GetViewport();
  const float zFar = cam.GetFarClipDistance();
#if defined(TARGET_PC)
  gpRender->SetPerspective(cam.GetFov(), cam.GetAspectRatio(), cam.GetNearClipDistance(), zFar);
#else
  gpRender->SetPerspective(cam.GetFov(), static_cast< float >(viewport.mWidth),
                           static_cast< float >(viewport.mHeight), cam.GetNearClipDistance(), zFar);
#endif
}

void CStateManager::DrawAdditionalFilters() const {
  if (mEscapeTimer < 1.f && mEscapeTimer > 0.f &&
      !mCameraManager->IsInCinematicCamera()) {
    const float alpha = 1.f - mEscapeTimer;
    const CColor color = CColor::White().WithAlphaOf(alpha);
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Add, CCameraFilterPass::kFS_Fullscreen,
                                  color, 0, 1.f);
  }
}

void CStateManager::DrawE3DeathEffect() const {
  const CPlayer* player = mPlayer;
  const float deathTime = player->GetDeathTime();
  float blurFactor;
  float whiteFactor;

  if (deathTime > 0.f) {
    if (player->GetMorphballTransitionState() != CPlayer::kMS_Unmorphed) {
      blurFactor = (deathTime - 1.f) / (gkBallDeathTime - 1.f);
      const float blurAmt = CMath::Clamp(0.f, blurFactor, 1.f);
      if (blurAmt > 0.f) {
        CCameraBlurPass blur;
        blur.SetBlur(CCameraBlurPass::kBT_HiBlur, 7.f * blurAmt, 0.f, false);
        blur.Draw();
      }
    }

    whiteFactor = 1.f - deathTime / (0.05f * gkBallDeathTime);
    const float whiteAmt = CMath::Clamp(0.f, whiteFactor, 1.f);
    const CColor color = CColor::White().WithAlphaOf(whiteAmt);
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Add, CCameraFilterPass::kFS_Fullscreen,
                                  color, 0, 1.f);
  }
}

void CStateManager::RenderCamerasAndAreaLights() const {
  mCameraManager->Render(*this);
  for (int i = 0; i < kCFS_Max; ++i) {
    mCamFilterPasses[i].Draw();
  }
}

void CStateManager::DrawDebugStuff() const {}

void CStateManager::BuildDynamicLightListForWorld() {
  if (mPlayerState->GetActiveVisor(*this) == CPlayerState::kPV_Thermal) {
    mDynamicLights = rstl::vector< CLight >();
    return;
  }

  const CObjectList& list = GetObjectListById(kOL_GameLight);
  int listSize = list.size();
  if (listSize == 0) {
    return;
  }

  if (mDynamicLights.capacity() != listSize) {
    mDynamicLights = rstl::vector< CLight >();
    mDynamicLights.reserve(listSize);
  } else {
    mDynamicLights.clear();
  }

  for (int idx = list.GetFirstObjectIndex(); idx != -1; idx = list.GetNextObjectIndex(idx)) {
    const CGameLight* light = static_cast< const CGameLight* >(list[idx]);
    if (light && light->GetActive()) {
      const CLight& l = light->GetLight();
      if (l.GetIntensity() > FLT_EPSILON && l.GetRadius() > FLT_EPSILON) {
        // TODO: This shouldn't be inlined, but currently is.
        mDynamicLights.push_back(l);
      }
    }
  }
  rstl::sort(mDynamicLights.begin(), mDynamicLights.end(), CLightPredicate());
}

bool CStateManager::CanCreateProjectile(TUniqueId uid, EWeaponType type, int maxAllowed) const {
  return mWeaponMgr->GetNumActive(uid, type) < maxAllowed;
}

void CStateManager::ReflectionDrawer(void* ctx, const CVector3f& point) {
  CStateManager* mgr = reinterpret_cast< CStateManager* >(ctx);
  mgr->DrawReflection(point);
}

void CStateManager::CacheReflection() {
  gpRender->CacheReflection(ReflectionDrawer, this, !gkWorldOnlyReflection);
}

void CStateManager::DrawReflection(const CVector3f& point) {
  CPlayer* player = mPlayer;
  CAABox playerBounds = player->GetBoundingBox();
  CVector3f playerPos = playerBounds.GetCenterPoint();

  const CVector3f viewPos =
      playerPos - 3.5f * CVector3f(playerPos.GetX() - point.GetX(), playerPos.GetY() - point.GetY(),
                                   playerPos.GetZ() - playerPos.GetZ())
                             .AsNormalized();

  CTransform4f reflectionXf = CTransform4f::LookAt(viewPos, playerPos, CVector3f(0.f, 0.f, -1.f));
  const CTransform4f backupView = CGraphics::GetViewMatrix();
  CGraphics::SetViewPointMatrix(reflectionXf);

  const CGameCamera& curCam = mCameraManager->GetCurrentCamera(*this);
  const CGraphics::CProjectionState backupProj = CGraphics::GetProjectionState();

  const CViewport& viewport = CGraphics::GetViewport();
  const float zFar = curCam.GetFarClipDistance();
  gpRender->SetPerspective(curCam.GetFov(), static_cast< float >(viewport.mWidth),
                           static_cast< float >(viewport.mHeight), curCam.GetNearClipDistance(),
                           zFar);

  player->RenderReflectedPlayer(*this);

  CGraphics::SetViewPointMatrix(backupView);
  CGraphics::SetProjectionState(backupProj);
}

void CStateManager::DrawSpaceWarp(const CVector3f& point, float strength) const {
  switch (mPlayerState->GetActiveVisor(*this)) {
  case CPlayerState::kPV_XRay:
  case CPlayerState::kPV_Thermal:
    return;
  default:
    break;
  }

  const CGameCamera& curCam = mCameraManager->GetCurrentCamera(*this);
  gpRender->DrawSpaceWarp(curCam.ConvertToScreenSpace(point), strength);
}

void CStateManager::TouchSky() const { mWorld->TouchSky(); }

void CStateManager::TouchPlayerActor() const {
  if (mPlayerActorHead != kInvalidUniqueId) {
    const CEntity* ent = GetObjectById(mPlayerActorHead);
    if (ent != nullptr) {
      static_cast< CScriptPlayerActor* >(const_cast< CEntity* >(ent))->TouchModels(*this);
    }
  }
}

void CStateManager::SetActorAreaId(CActor& actor, const TAreaId aid) {
  const int oldArea = actor.mAreaId.value;
  if (oldArea != aid.value) {
    CWorld* world = mWorld.get();
    if (oldArea != kInvalidAreaId.value) {
      CGameArea* oldAreaObj = world->Area(TAreaId(oldArea));
      if (oldAreaObj->IsPostConstructed()) {
        oldAreaObj->ObjectList()->RemoveObject(actor.GetUniqueId());
      }
    }

    actor.__SetCurrentAreaId(aid);
    if (aid != kInvalidAreaId) {
      CGameArea* newAreaObj = world->Area(aid);
      if (newAreaObj->IsPostConstructed()) {
        if (newAreaObj->GetObjectList()->GetValidObjectById(actor.GetUniqueId()) == nullptr) {
          newAreaObj->ObjectList()->AddObject(actor);
        }
      }
    }
  }
}

const rstl::string& CStateManager::HashInstanceName(CInputStream& in) {
  static rstl::string empty = rstl::string_l("");
  uchar n = 0;
  do {
    n = in.ReadChar();
  } while (n != '\0');
  return empty;
}

void CStateManager::MurderScriptInstanceNames() {
  bool done = false;

  while (!done) {
    done = true;

    for (rstl::set< rstl::string >::iterator it = mUniqueInstanceNames.begin();
         it != mUniqueInstanceNames.end(); ++it) {
      if (it->refcount() == 1) {
        mUniqueInstanceNames.erase(it);
        done = false;
        break;
      }
    }
  }
}

void CStateManager::SetupParticleHook(const CActor& actor) const {
  mActorModelParticles->SetupHook(actor.GetUniqueId());
}

void CStateManager::ResetEscapeSequenceTimer(float time) {
  mEscapeTimer = time;
  mEscapeTotalTime = time;
}

float CStateManager::GetEscapeSequenceTimer() const { return mEscapeTimer; }

void CStateManager::UpdateEscapeSequenceTimer(float dt) {
  const float totalTime = mEscapeTotalTime;
  if (mEscapeTimer > 0.f) {
    mEscapeTimer = rstl::max_val(gkEpsilon, mEscapeTimer - dt);
    if (mEscapeTimer <= FLT_EPSILON) {
      mPlayerState->SetPlayerAlive(false);
    }

    if (!init) {
      init = true;
      nextShake = 0.f;
    }

    nextShake -= dt;
    if (nextShake < 0.f) {
      const float factor = 1.f - mEscapeTimer / totalTime;
      const float factorSq = factor * factor;
      {
        const CCameraShakeData& shakeData =
            CCameraShakeData::HardVertShake(1.f, 0.2f * factorSq * mRandom->Range(0.5f, 1.f));
        mCameraManager->AddCameraShaker(shakeData, true);
      }
      mRumbleManager->Rumble(*this, static_cast< ERumbleFxId >(0xb), 0.75f, kRP_One);
      nextShake = -12.f * factorSq + 15.f;
    }
  }
}

void CStateManager::UpdateHintState(float dt) {
  CHintOptions& hintOpts = gpGameState->HintOptions();
  hintOpts.Update(dt, *this);

  int nextHintIdx = -1;
  int hintPeriods = -1;
  const SHintState* curHint = hintOpts.GetCurrentDisplayedHint();
  if (curHint != NULL) {
    const CGameHintInfo::CGameHint& nextHint = GetGameHints()[hintOpts.GetNextHintIdx()];
    const rstl::vector< CGameHintInfo::SHintLocation >& locations = nextHint.GetLocations();
    for (int i = 0; i < static_cast< int >(locations.size()); ++i) {
      const CGameHintInfo::SHintLocation& location = locations[i];
      const int areaId = location.mAreaId.Value();
      const CAssetId mlvlId = location.mMlvlId;
      CWorldState& worldState = gpGameState->StateForWorld(mlvlId);
      rstl::rc_ptr< CMapWorldInfo > mapWorldInfo = worldState.MapWorldInfo();
      mapWorldInfo->SetIsMapped(areaId, true);
    }

    if (curHint->mTime < nextHint.GetTextTime()) {
      nextHintIdx = hintOpts.GetNextHintIdx();
      hintPeriods = static_cast< int >(curHint->mTime / CGameHintInfo::skHintTextTime);
    }
  }

  if (nextHintIdx != mHintIdx || hintPeriods != static_cast< int >(mHintPeriods)) {
    if (nextHintIdx == -1) {
      const rstl::wstring& empty = rstl::wstring_l(L"");
      CHUDMemoParms memoInfo(0.f, true, true, true);
      CSamusHud::DisplayHudMemo(empty, memoInfo);
    } else {
      const CGameHintInfo::CGameHint* hint = &GetGameHints()[nextHintIdx];
      CHUDMemoParms memoInfo(0.f, true, false, true);
      CSamusHud::DeferHintMemo(hint->GetStringId(), hintPeriods, memoInfo);
    }

    mHintIdx = nextHintIdx;
    mHintPeriods = hintPeriods;
  }
}

void CStateManager::AddWeaponId(TUniqueId uid, EWeaponType type) {
  mWeaponMgr->IncrCount(uid, type);
}

void CStateManager::RemoveWeaponId(TUniqueId uid, EWeaponType type) {
  mWeaponMgr->DecrCount(uid, type);
}

int CStateManager::GetWeaponIdCount(TUniqueId uid, EWeaponType type) {
  return mWeaponMgr->GetNumActive(uid, type);
}

TAreaId CStateManager::GetVisAreaId() const {
  const CGameCamera& curCam = mCameraManager->GetCurrentCamera(*this);
  const CBallCamera* ballCam = mCameraManager->GetBallCamera();
  const TAreaId curArea = mWorld->GetCurrentAreaId();
  if (&curCam != ballCam) {
    return curArea;
  }

  const CVector3f camTranslation = ballCam->GetTranslation();
  const CVector3f playerTranslation = mPlayer->GetTranslation();
  CAABox camAABB(camTranslation, camTranslation);
  camAABB.AccumulateBounds(playerTranslation);

  TEntityList nearList;
  const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_AIBlock));
  BuildNearList(nearList, camAABB, filter, nullptr);
  for (const TUniqueId* uid = nearList.begin(); uid != nearList.end(); ++uid) {
    const CScriptDock* dock = TCastToConstPtr< CScriptDock >(GetObjectById(*uid));
    if (dock != nullptr && dock->GetAreaId() == curArea &&
        dock->HasPointCrossedDock(*this, camTranslation)) {
      return dock->GetCurrentConnectedAreaId(*this);
    }
  }

  return curArea;
}

bool CStateManager::SpecialSkipCinematic() {
  bool hadRandom;
  if (mSkipCineSpecialFunc == kInvalidUniqueId) {
    return false;
  }

  CScriptSpecialFunction* const special =
      static_cast< CScriptSpecialFunction* >(ObjectById(TUniqueId(mSkipCineSpecialFunc)));
  if (special == nullptr || !special->ShouldSkipCinematic(*this)) {
    return false;
  }

  hadRandom = mRandom != nullptr;
  mRandom = &mRandomValue;
  mCameraManager->StopCinematics(*this);
  special->SkipCinematic(*this);
  mRandom = hadRandom ? &mRandomValue : nullptr;
  return true;
}

void CStateManager::AddDrawableActor(const CActor& actor, const CVector3f& pos,
                                     const CAABox& bounds) const {
  const_cast< CActor& >(actor).SetAddedToken(mObjectDrawToken + 1);
  gpRender->AddDrawable(&actor, pos, bounds, 0, IRenderer::kDS_SortedCallback);
}

void CStateManager::AddDrawableActorPlane(const CActor& actor, const CPlane& plane,
                                          const CAABox& bounds) const {
  const_cast< CActor& >(actor).SetAddedToken(mObjectDrawToken + 1);
  gpRender->AddPlaneObject(&actor, bounds, plane, 0);
}

bool CStateManager::RenderLast(const TUniqueId& uid) {
  CStateManagerContainer* container = mStateManagerContainer.get();
  if (container->mRenderLast.size() == 20) {
    return false;
  }
  container->mRenderLast.push_back(uid);
  return true;
}

void CStateManager::DeferStateTransition(EStateManagerTransition t) {
  if (t == kSMT_InGame) {
    if (mDeferredTransition != kSMT_InGame) {
      mWorld->SetLoadPauseState(false);
      mDeferredTransition = kSMT_InGame;
    }
  } else if (mDeferredTransition == kSMT_InGame) {
    mWorld->SetLoadPauseState(true);
    mDeferredTransition = t;
  }
}

void CStateManager::ShowPausedHUDMemo(CAssetId strg, float time) {
  mHudMessageTime = time;
  mPauseHudMessage = strg;
  DeferStateTransition(kSMT_MessageScreen);
}

bool CStateManager::CanShowMapScreen() {
  const SHintState* curHint = gpGameState->HintOptions().GetCurrentDisplayedHint();
  if (curHint != nullptr && !const_cast< SHintState* >(curHint)->CanContinue()) {
    return false;
  }
  return true;
}

void CStateManager::UpdateThermalVisor() {
  mThermColdScale2 = 0.f;
  mThermColdScale1 = 0.f;

  if (mPlayerState->GetActiveVisor(*this) == CPlayerState::kPV_Thermal) {
    const TAreaId nextArea = GetNextAreaId();
    if (nextArea != kInvalidAreaId) {
      CGameArea* bestArea = nullptr;
      CGameArea* const curArea = mWorld->Area(nextArea);
      const CVector3f playerPos = mPlayer->GetTranslation();
      float bestDistSq = FLT_MAX;
      const int dockCount = curArea->GetDockCount();
      for (int i = 0; i < dockCount; ++i) {
        const IGameArea::Dock& dock = curArea->GetDock(i);
        const rstl::reserved_vector< CVector3f, 4 >& verts = dock.GetPlaneVertices();
        const CVector3f dockCenter = 0.25f * (verts[0] + verts[1] + verts[2] + verts[3]);
        const float distSq = CVector3f(playerPos.GetX() - dockCenter.GetX(),
                                       playerPos.GetY() - dockCenter.GetY(), 0.f)
                                 .MagSquared();
        if (distSq < bestDistSq) {
          const TAreaId connectedAreaId = dock.GetConnectedAreaId(0);
          if (connectedAreaId != kInvalidAreaId) {
            CGameArea* connectedArea = mWorld->Area(connectedAreaId);
            if (connectedArea->IsPostConstructed() &&
                connectedArea->GetOcclusionState() == CGameArea::kOS_Visible) {
              bestDistSq = distSq;
              bestArea = connectedArea;
            }
          }
        }
      }

      const float areaThermal = curArea->GetPostConstructed()->mThermalCurrent;
      if (bestArea != nullptr) {
        float dist = CMath::FastSqrtF(bestDistSq) - 2.f;
        if (dist < 8.f) {
          if (dist > 0.f) {
            dist = 0.5f * (0.125f * dist) + 0.5f;
          } else {
            dist = 0.5f;
          }
          mThermColdScale1 =
              dist * areaThermal +
              (1.f - dist) * bestArea->GetPostConstructed()->mThermalCurrent;
        } else {
          mThermColdScale1 = areaThermal;
        }
      } else {
        mThermColdScale1 = areaThermal;
      }
    }
  }
}

void CStateManager::GetCharacterRenderMaskAndTarget(bool thawed, int& mask, int& target) {
  const CPlayerState::EPlayerVisor visor = mPlayerState->GetActiveVisor(*this);
  int renderMask = 0;
  int renderTarget = 0;
  switch (visor) {
  case CPlayerState::kPV_Combat:
  case CPlayerState::kPV_Scan:
    renderMask = 0x1000;
    renderTarget = 0;
    break;
  case CPlayerState::kPV_XRay:
    renderMask = 0x800;
    renderTarget = 0;
    break;
  case CPlayerState::kPV_Thermal:
    if (thawed) {
      if (mThermalFlag == kTD_Hot) {
        renderMask = 0x600;
        renderTarget = 0;
      } else {
        renderMask = 0x600;
        renderTarget = 0x200;
      }
    } else {
      if (mThermalFlag == kTD_Cold) {
        renderMask = 0x500;
        renderTarget = 0;
      } else {
        renderMask = 0x500;
        renderTarget = 0x100;
      }
    }
    break;
  default:
    break;
  }
  mask = renderMask;
  target = renderTarget;
}

void CStateManager::DoCollisionResponse(const CCollisionResponseData& colRespData,
                                        const CRayCastResult& rayCast, const TUniqueId uid,
                                        const CWeaponMode& weaponMode, bool w1, bool b1) {
  if (!rayCast.IsValid()) {
    return;
  }

  CTransform4f xf = CTransform4f::LookAt(CVector3f::Zero(), rayCast.GetPlane().GetNormal());
  xf.SetTranslation(rayCast.GetPoint());

  EWeaponCollisionResponseTypes type = kWCR_Default;
  if (uid == kInvalidUniqueId) {
    type = CCollisionResponseData::GetWorldCollisionResponseType(
        CMaterialList::BitPosition(rayCast.GetMaterial().GetValue() & 0x00e3fffe));
  } else if (const CActor* actor = TCastToConstPtr< CActor >(GetObjectById(uid))) {
    type = actor->GetCollisionResponseType(rayCast.GetPoint(), rayCast.GetPlane().GetNormal(),
                                           weaponMode, 0);
  }

  rstl::optional_object< TLockedToken< CGenDescription > > particleDesc =
      colRespData.GetParticleDescription(type);

  if (particleDesc.valid()) {
    CExplosion* const explosion = rs_new CExplosion(
        *particleDesc, AllocateUniqueId(), true,
        CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList, kInvalidEditorId),
        rstl::string_l("Proj col resp"), xf, (b1 ? 0u : 1u) | (w1 ? 2u : 0u),
        CVector3f(1.f, 1.f, 1.f), CColor::White());
    AddObject(explosion);
  }

  int soundId = colRespData.GetSoundEffectId(type);
  if (soundId >= 0) {
    float range = colRespData.GetAudibleRange();
    float fallOff = colRespData.GetAudibleFallOff();
    CAudioSys::C3DEmitterParmData parmData(range, fallOff, 1, CAudioSys::kMaxVolume, 0x14);
    parmData.mPos = rayCast.GetPoint();
    parmData.mSfxId = CSfxManager::TranslateSFXID(static_cast< ushort >(soundId));
    CSfxManager::AddEmitter(parmData, true, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
  }
}

const CSinglePathMaze* CStateManager::GetSinglePathMaze() const { return mCurrentMaze.get(); }

CSinglePathMaze* CStateManager::SinglePathMaze() { return mCurrentMaze.get(); }

void CStateManager::SetSinglePathMaze(rstl::single_ptr< CSinglePathMaze > maze) {
  mCurrentMaze = maze;
}

void CStateManager::SetPendingOnScreenTex(CAssetId texId, const CVector2i& extent,
                                          const CVector2i& offset) {
  mPendingScreenTex.mId = texId;
  mPendingScreenTex.mExtent = extent;
  mPendingScreenTex.mOffset = offset;
}

void CStateManager::SetGameState(EGameState state) {
  if (mGameState == state) {
    return;
  }

  if (mGameState == kGS_SoftPaused) {
    mWorld->SetLoadPauseState(false);
  }

  switch (state) {
  case kGS_Running: {
    CRumbleGenerator* rumbleGen = reinterpret_cast< CRumbleGenerator* >(mRumbleManager);
    if (rumbleGen->GetDisabled()) {
      rumbleGen->SetDisabled(false);
    }
    break;
  }
  case kGS_SoftPaused: {
    CRumbleGenerator* rumbleGen = reinterpret_cast< CRumbleGenerator* >(mRumbleManager);
    if (!rumbleGen->GetDisabled()) {
      rumbleGen->SetDisabled(true);
    }
    mWorld->SetLoadPauseState(true);
    break;
  }
  default:
    break;
  }

  mGameState = state;
}

void CStateManager::SetBossParams(TUniqueId bossId, float maxEnergy, uint stringIdx) {
  mBossId = bossId;
  mTotalBossEnergy = maxEnergy;
  mBossStringIdx = stringIdx;
}

#if !defined(TARGET_PC)
const CPlayer* CStateManager::GetPlayer() const { return mPlayer; }
#endif

void CStateManager::QueueMessage(int frameCount, CAssetId msg, float f1) {
  xf84_ = frameCount;
  xf88_ = msg;
  xf8c_ = f1;
}

void CStateManager::DeliverScriptMsg(CEntity* ent, TUniqueId target, EScriptObjectMessage msg) {
  if (ent != nullptr && !ent->IsScriptingBlocked()) {
    ent->AcceptScriptMsg(msg, target, *this);
  }
}

void CStateManager::SendScriptMsgAlways(TUniqueId uid, TUniqueId src, EScriptObjectMessage msg) {
  CEntity* ent = ObjectById(uid);
  if (ent != nullptr) {
    ent->AcceptScriptMsg(msg, src, *this);
  }
}

float CStateManager::IntegrateVisorFog(float f) const {
  if (mPlayerState->GetActiveVisor(*this) == CPlayerState::kPV_Scan) {
    return f * (1.f - mPlayerState->GetVisorTransitionFactor());
  }
  return f;
}

#if VERSION >= VERSION_GM8P_00
void CStateManager::SetProjectedShadow(CProjectedShadow* shadow) {
  shadow->SetNextShadow(mProjectedShadow);
  mProjectedShadow = shadow;
}
#endif
