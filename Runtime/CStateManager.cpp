#include "CStateManager.hpp"
#include "Camera/CCameraShakeData.hpp"
#include "CSortedLists.hpp"
#include "CWeaponMgr.hpp"
#include "CFluidPlaneManager.hpp"
#include "World/CEnvFxManager.hpp"
#include "World/CActorModelParticles.hpp"
#include "World/CTeamAiTypes.hpp"
#include "Input/CRumbleManager.hpp"
#include "World/CWorld.hpp"
#include "Graphics/CLight.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CPlayerState.hpp"
#include "World/CPlayer.hpp"
#include "World/CMorphBall.hpp"

namespace urde
{

CStateManager::CStateManager(const std::weak_ptr<CScriptMailbox>&,
                             const std::weak_ptr<CMapWorldInfo>&,
                             const std::weak_ptr<CPlayerState>&,
                             const std::weak_ptr<CWorldTransManager>&)
: x80c_allObjs(new CObjectList(EGameObjectList::All)),
  x814_actorObjs(new CActorList()),
  x81c_physActorObjs(new CPhysicsActorList()),
  x824_cameraObjs(new CGameCameraList()),
  x82c_lightObjs(new CGameLightList()),
  x834_listenAiObjs(new CListeningAiList()),
  x83c_aiWaypointObjs(new CAiWaypointList()),
  x844_platformAndDoorObjs(new CPlatformAndDoorList()),
  x870_cameraManager(new CCameraManager(kInvalidUniqueId)),
  x874_sortedListManager(new CSortedListManager()),
  x878_weaponManager(new CWeaponMgr()),
  x87c_fluidPlaneManager(new CFluidPlaneManager()),
  x880_envFxManager(new CEnvFxManager()),
  x884_actorModelParticles(new CActorModelParticles()),
  x888_teamAiTypes(new CTeamAiTypes()),
  x88c_rumbleManager(new CRumbleManager())
{
    x904_loaderFuncs[int(EScriptObjectType::Actor)] = ScriptLoader::LoadActor;
    x904_loaderFuncs[int(EScriptObjectType::Waypoint)] = ScriptLoader::LoadWaypoint;
    x904_loaderFuncs[int(EScriptObjectType::Door)] = ScriptLoader::LoadDoor;
    x904_loaderFuncs[int(EScriptObjectType::Trigger)] = ScriptLoader::LoadTrigger;
    x904_loaderFuncs[int(EScriptObjectType::Timer)] = ScriptLoader::LoadTimer;
    x904_loaderFuncs[int(EScriptObjectType::Counter)] = ScriptLoader::LoadCounter;
    x904_loaderFuncs[int(EScriptObjectType::Effect)] = ScriptLoader::LoadEffect;
    x904_loaderFuncs[int(EScriptObjectType::Platform)] = ScriptLoader::LoadPlatform;
    x904_loaderFuncs[int(EScriptObjectType::Sound)] = ScriptLoader::LoadSound;
    x904_loaderFuncs[int(EScriptObjectType::Generator)] = ScriptLoader::LoadGenerator;
    x904_loaderFuncs[int(EScriptObjectType::Camera)] = ScriptLoader::LoadCamera;
    x904_loaderFuncs[int(EScriptObjectType::CameraWaypoint)] = ScriptLoader::LoadCameraWaypoint;
    x904_loaderFuncs[int(EScriptObjectType::NewIntroBoss)] = ScriptLoader::LoadNewIntroBoss;
    x904_loaderFuncs[int(EScriptObjectType::SpawnPoint)] = ScriptLoader::LoadSpawnPoint;
    x904_loaderFuncs[int(EScriptObjectType::CameraHint)] = ScriptLoader::LoadCameraHint;
    x904_loaderFuncs[int(EScriptObjectType::Pickup)] = ScriptLoader::LoadPickup;
    x904_loaderFuncs[int(EScriptObjectType::MemoryRelay)] = ScriptLoader::LoadMemoryRelay;
    x904_loaderFuncs[int(EScriptObjectType::RandomRelay)] = ScriptLoader::LoadRandomRelay;
    x904_loaderFuncs[int(EScriptObjectType::Relay)] = ScriptLoader::LoadRelay;
    x904_loaderFuncs[int(EScriptObjectType::Beetle)] = ScriptLoader::LoadBeetle;
    x904_loaderFuncs[int(EScriptObjectType::HUDMemo)] = ScriptLoader::LoadHUDMemo;
    x904_loaderFuncs[int(EScriptObjectType::CameraFilterKeyframe)] = ScriptLoader::LoadCameraFilterKeyframe;
    x904_loaderFuncs[int(EScriptObjectType::CameraBlurKeyframe)] = ScriptLoader::LoadCameraBlurKeyframe;
    x904_loaderFuncs[int(EScriptObjectType::DamageableTrigger)] = ScriptLoader::LoadDamageableTrigger;
    x904_loaderFuncs[int(EScriptObjectType::Debris)] = ScriptLoader::LoadDebris;
    x904_loaderFuncs[int(EScriptObjectType::CameraShaker)] = ScriptLoader::LoadCameraShaker;
    x904_loaderFuncs[int(EScriptObjectType::ActorKeyframe)] = ScriptLoader::LoadActorKeyframe;
    x904_loaderFuncs[int(EScriptObjectType::Water)] = ScriptLoader::LoadWater;
    x904_loaderFuncs[int(EScriptObjectType::Warwasp)] = ScriptLoader::LoadWarWasp;
    x904_loaderFuncs[int(EScriptObjectType::SpacePirate)] = ScriptLoader::LoadSpacePirate;
    x904_loaderFuncs[int(EScriptObjectType::FlyingPirate)] = ScriptLoader::LoadFlyingPirate;
    x904_loaderFuncs[int(EScriptObjectType::ElitePirate)] = ScriptLoader::LoadElitePirate;
    x904_loaderFuncs[int(EScriptObjectType::MetroidBeta)] = ScriptLoader::LoadMetroidBeta;
    x904_loaderFuncs[int(EScriptObjectType::ChozoGhost)] = ScriptLoader::LoadChozoGhost;
    x904_loaderFuncs[int(EScriptObjectType::CoverPoint)] = ScriptLoader::LoadCoverPoint;
    x904_loaderFuncs[int(EScriptObjectType::SpiderBallWaypoint)] = ScriptLoader::LoadSpiderBallWaypoint;
    x904_loaderFuncs[int(EScriptObjectType::BloodFlower)] = ScriptLoader::LoadBloodFlower;
    x904_loaderFuncs[int(EScriptObjectType::FlickerBat)] = ScriptLoader::LoadFlickerBat;
    x904_loaderFuncs[int(EScriptObjectType::PathCamera)] = ScriptLoader::LoadPathCamera;
    x904_loaderFuncs[int(EScriptObjectType::GrapplePoint)] = ScriptLoader::LoadGrapplePoint;
    x904_loaderFuncs[int(EScriptObjectType::PuddleSpore)] = ScriptLoader::LoadPuddleSpore;
    x904_loaderFuncs[int(EScriptObjectType::DebugCameraWaypoint)] = ScriptLoader::LoadDebugCameraWaypoint;
    x904_loaderFuncs[int(EScriptObjectType::SpiderBallAttractionSurface)] = ScriptLoader::LoadSpiderBallAttractionSurface;
    x904_loaderFuncs[int(EScriptObjectType::PuddleToadGamma)] = ScriptLoader::LoadPuddleToadGamma;
    x904_loaderFuncs[int(EScriptObjectType::DistanceFog)] = ScriptLoader::LoadDistanceFog;
    x904_loaderFuncs[int(EScriptObjectType::FireFlea)] = ScriptLoader::LoadFireFlea;
    x904_loaderFuncs[int(EScriptObjectType::MetareeAlpha)] = ScriptLoader::LoadMetareeAlpha;
    x904_loaderFuncs[int(EScriptObjectType::DockAreaChange)] = ScriptLoader::LoadDockAreaChange;
    x904_loaderFuncs[int(EScriptObjectType::ActorRotate)] = ScriptLoader::LoadActorRotate;
    x904_loaderFuncs[int(EScriptObjectType::SpecialFunction)] = ScriptLoader::LoadSpecialFunction;
    x904_loaderFuncs[int(EScriptObjectType::SpankWeed)] = ScriptLoader::LoadSpankWeed;
    x904_loaderFuncs[int(EScriptObjectType::Parasite)] = ScriptLoader::LoadParasite;
    x904_loaderFuncs[int(EScriptObjectType::PlayerHint)] = ScriptLoader::LoadPlayerHint;
    x904_loaderFuncs[int(EScriptObjectType::Ripper)] = ScriptLoader::LoadRipper;
    x904_loaderFuncs[int(EScriptObjectType::PickupGenerator)] = ScriptLoader::LoadPickupGenerator;
    x904_loaderFuncs[int(EScriptObjectType::AIKeyframe)] = ScriptLoader::LoadAIKeyframe;
    x904_loaderFuncs[int(EScriptObjectType::PointOfInterest)] = ScriptLoader::LoadPointOfInterest;
    x904_loaderFuncs[int(EScriptObjectType::Drone)] = ScriptLoader::LoadDrone;
    x904_loaderFuncs[int(EScriptObjectType::MetroidAlpha)] = ScriptLoader::LoadMetroidAlpha;
    x904_loaderFuncs[int(EScriptObjectType::DebrisExtended)] = ScriptLoader::LoadDebrisExtended;
    x904_loaderFuncs[int(EScriptObjectType::Steam)] = ScriptLoader::LoadSteam;
    x904_loaderFuncs[int(EScriptObjectType::Ripple)] = ScriptLoader::LoadRipple;
    x904_loaderFuncs[int(EScriptObjectType::BallTrigger)] = ScriptLoader::LoadBallTrigger;
    x904_loaderFuncs[int(EScriptObjectType::TargetingPoint)] = ScriptLoader::LoadTargetingPoint;
    x904_loaderFuncs[int(EScriptObjectType::EMPulse)] = ScriptLoader::LoadEMPulse;
    x904_loaderFuncs[int(EScriptObjectType::IceSheegoth)] = ScriptLoader::LoadIceSheegoth;
    x904_loaderFuncs[int(EScriptObjectType::PlayerActor)] = ScriptLoader::LoadPlayerActor;
    x904_loaderFuncs[int(EScriptObjectType::Flaahgra)] = ScriptLoader::LoadFlaahgra;
    x904_loaderFuncs[int(EScriptObjectType::AreaAttributes)] = ScriptLoader::LoadAreaAttributes;
    x904_loaderFuncs[int(EScriptObjectType::FishCloud)] = ScriptLoader::LoadFishCloud;
    x904_loaderFuncs[int(EScriptObjectType::FishCloudModifier)] = ScriptLoader::LoadFishCloudModifier;
    x904_loaderFuncs[int(EScriptObjectType::VisorFlare)] = ScriptLoader::LoadVisorFlare;
    x904_loaderFuncs[int(EScriptObjectType::WorldTeleporterx52)] = ScriptLoader::LoadWorldTeleporter;
    x904_loaderFuncs[int(EScriptObjectType::VisorGoo)] = ScriptLoader::LoadVisorGoo;
    x904_loaderFuncs[int(EScriptObjectType::JellyZap)] = ScriptLoader::LoadJellyZap;
    x904_loaderFuncs[int(EScriptObjectType::ControllerAction)] = ScriptLoader::LoadControllerAction;
    x904_loaderFuncs[int(EScriptObjectType::Switch)] = ScriptLoader::LoadSwitch;
    x904_loaderFuncs[int(EScriptObjectType::PlayerStateChange)] = ScriptLoader::LoadPlayerStateChange;
    x904_loaderFuncs[int(EScriptObjectType::Thardus)] = ScriptLoader::LoadThardus;
    x904_loaderFuncs[int(EScriptObjectType::WallCrawlerSwarm)] = ScriptLoader::LoadWallCrawlerSwarm;
    x904_loaderFuncs[int(EScriptObjectType::AIJumpPoint)] = ScriptLoader::LoadAIJumpPoint;
    x904_loaderFuncs[int(EScriptObjectType::FlaahgraTentacle)] = ScriptLoader::LoadFlaahgraTentacle;
    x904_loaderFuncs[int(EScriptObjectType::RoomAcoustics)] = ScriptLoader::LoadRoomAcoustics;
    x904_loaderFuncs[int(EScriptObjectType::ColorModulate)] = ScriptLoader::LoadColorModulate;
    x904_loaderFuncs[int(EScriptObjectType::ThardusRockProjectile)] = ScriptLoader::LoadThardusRockProjectile;
    x904_loaderFuncs[int(EScriptObjectType::Midi)] = ScriptLoader::LoadMidi;
    x904_loaderFuncs[int(EScriptObjectType::StreamedAudio)] = ScriptLoader::LoadStreamedAudio;
    x904_loaderFuncs[int(EScriptObjectType::WorldTeleporterx62)] = ScriptLoader::LoadWorldTeleporter;
    x904_loaderFuncs[int(EScriptObjectType::Repulsor)] = ScriptLoader::LoadRepulsor;
    x904_loaderFuncs[int(EScriptObjectType::GunTurret)] = ScriptLoader::LoadGunTurret;
    x904_loaderFuncs[int(EScriptObjectType::FogVolume)] = ScriptLoader::LoadFogVolume;
    x904_loaderFuncs[int(EScriptObjectType::Babygoth)] = ScriptLoader::LoadBabygoth;
    x904_loaderFuncs[int(EScriptObjectType::Eyeball)] = ScriptLoader::LoadEyeball;
    x904_loaderFuncs[int(EScriptObjectType::RadialDamage)] = ScriptLoader::LoadRadialDamage;
    x904_loaderFuncs[int(EScriptObjectType::CameraPitchVolume)] = ScriptLoader::LoadCameraPitchVolume;
    x904_loaderFuncs[int(EScriptObjectType::EnvFxDensityController)] = ScriptLoader::LoadEnvFxDensityController;
    x904_loaderFuncs[int(EScriptObjectType::Magdolite)] = ScriptLoader::LoadMagdolite;
    x904_loaderFuncs[int(EScriptObjectType::TeamAIMgr)] = ScriptLoader::LoadTeamAIMgr;
    x904_loaderFuncs[int(EScriptObjectType::SnakeWeedSwarm)] = ScriptLoader::LoadSnakeWeedSwarm;
    x904_loaderFuncs[int(EScriptObjectType::ActorContraption)] = ScriptLoader::LoadActorContraption;
    x904_loaderFuncs[int(EScriptObjectType::Oculus)] = ScriptLoader::LoadOculus;
    x904_loaderFuncs[int(EScriptObjectType::Geemer)] = ScriptLoader::LoadGeemer;
    x904_loaderFuncs[int(EScriptObjectType::SpindleCamera)] = ScriptLoader::LoadSpindleCamera;
    x904_loaderFuncs[int(EScriptObjectType::AtomicAlpha)] = ScriptLoader::LoadAtomicAlpha;
    x904_loaderFuncs[int(EScriptObjectType::CameraHintTrigger)] = ScriptLoader::LoadCameraHintTrigger;
    x904_loaderFuncs[int(EScriptObjectType::RumbleEffect)] = ScriptLoader::LoadRumbleEffect;
    x904_loaderFuncs[int(EScriptObjectType::AmbientAI)] = ScriptLoader::LoadAmbientAI;
    x904_loaderFuncs[int(EScriptObjectType::AtomicBeta)] = ScriptLoader::LoadAtomicBeta;
    x904_loaderFuncs[int(EScriptObjectType::IceZoomer)] = ScriptLoader::LoadIceZoomer;
    x904_loaderFuncs[int(EScriptObjectType::Puffer)] = ScriptLoader::LoadPuffer;
    x904_loaderFuncs[int(EScriptObjectType::Tryclops)] = ScriptLoader::LoadTryclops;
    x904_loaderFuncs[int(EScriptObjectType::Ridley)] = ScriptLoader::LoadRidley;
    x904_loaderFuncs[int(EScriptObjectType::Seedling)] = ScriptLoader::LoadSeedling;
    x904_loaderFuncs[int(EScriptObjectType::ThermalHeatFader)] = ScriptLoader::LoadThermalHeatFader;
    x904_loaderFuncs[int(EScriptObjectType::Burrower)] = ScriptLoader::LoadBurrower;
    x904_loaderFuncs[int(EScriptObjectType::ScriptBeam)] = ScriptLoader::LoadScriptBeam;
    x904_loaderFuncs[int(EScriptObjectType::WorldLightFader)] = ScriptLoader::LoadWorldLightFader;
    x904_loaderFuncs[int(EScriptObjectType::MetroidPrimeStage2)] = ScriptLoader::LoadMetroidPrimeStage2;
    x904_loaderFuncs[int(EScriptObjectType::MetroidPrimeStage1)] = ScriptLoader::LoadMetroidPrimeStage1;
    x904_loaderFuncs[int(EScriptObjectType::MazeNode)] = ScriptLoader::LoadMazeNode;
    x904_loaderFuncs[int(EScriptObjectType::OmegaPirate)] = ScriptLoader::LoadOmegaPirate;
    x904_loaderFuncs[int(EScriptObjectType::PhazonPool)] = ScriptLoader::LoadPhazonPool;
    x904_loaderFuncs[int(EScriptObjectType::PhazonHealingNodule)] = ScriptLoader::LoadPhazonHealingNodule;
    x904_loaderFuncs[int(EScriptObjectType::NewCameraShaker)] = ScriptLoader::LoadNewCameraShaker;
    x904_loaderFuncs[int(EScriptObjectType::ShadowProjector)] = ScriptLoader::LoadShadowProjector;
    x904_loaderFuncs[int(EScriptObjectType::EnergyBall)] = ScriptLoader::LoadEnergyBall;

    x8ec_shadowTex = g_SimplePool->GetObj("DefaultShadow");
}

void CStateManager::UpdateThermalVisor()
{
    xf28_thermColdScale2 = 0.f;
    xf24_thermColdScale1 = 0.f;
    CPlayerState::EPlayerVisor visor = x8b8_playerState->GetActiveVisor(*this);
    if (visor == CPlayerState::EPlayerVisor::Thermal && x8cc_nextAreaId != kInvalidAreaId)
    {
        std::unique_ptr<CGameArea>& area = x850_world->GetGameAreas()[x8cc_nextAreaId];
        const zeus::CTransform& playerXf = x84c_player->GetTransform();
        float f30 = playerXf.origin.x;
        float f29 = playerXf.origin.y;
    }
}

void CStateManager::RenderLast(TUniqueId)
{
}

void CStateManager::AddDrawableActor(const CActor& actor, const zeus::CVector3f& vec,
                                     const zeus::CAABox& aabb) const
{
}

void CStateManager::SpecialSkipCinematic()
{
}

void CStateManager::GetVisAreaId() const
{
}

void CStateManager::GetWeaponIdCount(TUniqueId, EWeaponType)
{
}

void CStateManager::RemoveWeaponId(TUniqueId, EWeaponType)
{
}

void CStateManager::AddWeaponId(TUniqueId, EWeaponType)
{
}

void CStateManager::UpdateEscapeSequenceTimer(float)
{
}

float CStateManager::GetEscapeSequenceTimer() const
{
    return 0.f;
}

void CStateManager::ResetEscapeSequenceTimer(float)
{
}

void CStateManager::SetupParticleHook(const CActor& actor) const
{
}

void CStateManager::MurderScriptInstanceNames()
{
}

const std::string* CStateManager::HashInstanceName(CInputStream& in)
{
    return nullptr;
}

void CStateManager::SetActorAreaId(CActor& actor, TAreaId)
{
}

void CStateManager::TouchSky() const
{
}

void CStateManager::DrawSpaceWarp(const zeus::CVector3f&, float) const
{
}

void CStateManager::DrawReflection(const zeus::CVector3f&)
{
}

void CStateManager::CacheReflection()
{
}

bool CStateManager::CanCreateProjectile(TUniqueId, EWeaponType, int) const
{
    return false;
}

const CGameLightList* CStateManager::GetDynamicLightList() const
{
    return nullptr;
}

void CStateManager::BuildDynamicLightListForWorld(std::vector<CLight>& listOut) const
{
}

void CStateManager::DrawDebugStuff() const
{
}

void CStateManager::RenderCamerasAndAreaLights() const
{
}

void CStateManager::DrawE3DeathEffect() const
{
}

void CStateManager::DrawAdditionalFilters() const
{
}

void CStateManager::DrawWorld() const
{
}

void CStateManager::SetupFogForArea(const CGameArea& area) const
{
}

void CStateManager::PreRender()
{
}

void CStateManager::GetVisSetForArea(TAreaId, TAreaId) const
{
}

void CStateManager::RecursiveDrawTree(TUniqueId) const
{
}

void CStateManager::SendScriptMsg(TUniqueId uid, TEditorId eid,
                                  EScriptObjectMessage msg, EScriptObjectState state)
{
}

void CStateManager::FreeScriptObjects(TAreaId)
{
}

void CStateManager::GetBuildForScript(TEditorId) const
{
}

TEditorId CStateManager::GetEditorIdForUniqueId(TUniqueId) const
{
    return 0;
}

TUniqueId CStateManager::GetIdForScript(TEditorId) const
{
    return 0;
}

void CStateManager::GetIdListForScript(TEditorId) const
{
}

void CStateManager::LoadScriptObjects(TAreaId, CInputStream& in, EScriptPersistence)
{
}

void CStateManager::LoadScriptObject(TAreaId, EScriptObjectType, u32,
                                     CInputStream& in, EScriptPersistence)
{
}

void CStateManager::InformListeners(const zeus::CVector3f&, EListenNoiseType)
{
}

void CStateManager::ApplyKnockBack(CActor& actor, const CDamageInfo& info,
                                   const CDamageVulnerability&, const zeus::CVector3f&, float)
{
}

void CStateManager::ApplyDamageToWorld(TUniqueId, const CActor&, const zeus::CVector3f&,
                                       const CDamageInfo& info, const CMaterialFilter&)
{
}

void CStateManager::ProcessRadiusDamage(const CActor&, CActor&, const zeus::CVector3f&,
                                        const CDamageInfo& info, const CMaterialFilter&)
{
}

void CStateManager::ApplyRadiusDamage(const CActor&, const zeus::CVector3f&, CActor&,
                                      const CDamageInfo& info)
{
}

void CStateManager::ApplyLocalDamage(const zeus::CVector3f&, const zeus::CVector3f&, CActor&, float,
                                     const CWeaponMode&)
{
}

void CStateManager::ApplyDamage(TUniqueId, TUniqueId, TUniqueId, const CDamageInfo& info,
                                const CMaterialFilter&)
{
}

void CStateManager::UpdateAreaSounds()
{
}

void CStateManager::FrameEnd()
{
}

void CStateManager::ProcessPlayerInput()
{
}

void CStateManager::ProcessInput(const CFinalInput& input)
{
}

void CStateManager::Update(float dt)
{
}

void CStateManager::UpdateGameState()
{
}

void CStateManager::FrameBegin()
{
}

void CStateManager::InitializeState(u32, TAreaId, u32)
{
}

void CStateManager::CreateStandardGameObjects()
{
}

CObjectList* CStateManager::ObjectListById(EGameObjectList type)
{
    std::unique_ptr<CObjectList>* lists = &x80c_allObjs;
    return lists[int(type)].get();
}

const CObjectList* CStateManager::GetObjectListById(EGameObjectList type) const
{
    const std::unique_ptr<CObjectList>* lists = &x80c_allObjs;
    return lists[int(type)].get();
}

void CStateManager::RemoveObject(TUniqueId)
{
}

void CStateManager::UpdateRoomAcoustics(TAreaId)
{
}

void CStateManager::SetCurrentAreaId(TAreaId)
{
}

void CStateManager::ClearGraveyard()
{
}

void CStateManager::DeleteObjectRequest(TUniqueId)
{
}

CEntity* CStateManager::ObjectById(TUniqueId uid)
{
    return x80c_allObjs->GetObjectById(uid);
}
const CEntity* CStateManager::GetObjectById(TUniqueId uid) const
{
    return x80c_allObjs->GetObjectById(uid);
}

void CStateManager::AreaUnloaded(TAreaId)
{
}

void CStateManager::PrepareAreaUnload(TAreaId)
{
}

void CStateManager::AreaLoaded(TAreaId)
{
}

void CStateManager::BuildNearList(rstl::reserved_vector<TUniqueId, 1024>& listOut,
                                  const zeus::CVector3f&, const zeus::CVector3f&, float,
                                  const CMaterialFilter&, const CActor*) const
{
}

void CStateManager::BuildColliderList(rstl::reserved_vector<TUniqueId, 1024>& listOut,
                                      const CActor&, const zeus::CAABox&) const
{
}

void CStateManager::BuildNearList(rstl::reserved_vector<TUniqueId, 1024>& listOut,
                                  const zeus::CAABox&, const CMaterialFilter&, const CActor*) const
{
}

void CStateManager::UpdateActorInSortedLists(CActor&)
{
}

void CStateManager::UpdateSortedLists()
{
}

zeus::CAABox CStateManager::CalculateObjectBounds(const CActor&)
{
    return {};
}

void CStateManager::AddObject(CEntity&, EScriptPersistence)
{
}

void CStateManager::AddObject(CEntity*, EScriptPersistence)
{
}

bool CStateManager::RayStaticIntersection(const zeus::CVector3f&, const zeus::CVector3f&, float,
                                          const CMaterialFilter&) const
{
    return false;
}

bool CStateManager::RayWorldIntersection(TUniqueId, const zeus::CVector3f&, const zeus::CVector3f&,
                                         float, const CMaterialFilter&,
                                         const rstl::reserved_vector<TUniqueId, 1024>& list) const
{
    return false;
}

void CStateManager::UpdateObjectInLists(CEntity&)
{
}

TUniqueId CStateManager::AllocateUniqueId()
{
    return 0;
}

}
