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
#include "CGameState.hpp"
#include "World/CPlayer.hpp"
#include "World/CPlayerGun.hpp"
#include "World/CMorphBall.hpp"
#include "World/CScriptSpawnPoint.hpp"
#include "AutoMapper/CMapWorldInfo.hpp"

#include <cmath>

namespace urde
{

CStateManager::CStateManager(const std::weak_ptr<CRelayTracker>&,
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
  x844_platformAndDoorObjs(new CPlatformAndDoorList())
{
    x86c_stateManagerContainer.reset(new CStateManagerContainer);
    x870_cameraManager = &x86c_stateManagerContainer->x0_cameraManager;
    x874_sortedListManager = &x86c_stateManagerContainer->x3c0_sortedListManager;
    x878_weaponManager = &x86c_stateManagerContainer->xe3d8_weaponManager;
    x87c_fluidPlaneManager = &x86c_stateManagerContainer->xe3ec_fluidPlaneManager;
    x880_envFxManager = &x86c_stateManagerContainer->xe510_envFxManager;
    x884_actorModelParticles = &x86c_stateManagerContainer->xf168_actorModelParticles;
    x88c_rumbleManager = &x86c_stateManagerContainer->xf250_rumbleManager;

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
    x904_loaderFuncs[int(EScriptObjectType::WorldTeleporter)] = ScriptLoader::LoadWorldTeleporter;
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
    x904_loaderFuncs[int(EScriptObjectType::WorldTeleporterToo)] = ScriptLoader::LoadWorldTeleporter;
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

    x8f0_shadowTex = g_SimplePool->GetObj("DefaultShadow");
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
        zeus::CVector3f playerXYPos(playerXf.origin.x, playerXf.origin.y, 0.f);
        CGameArea* lastArea = nullptr;
        float closestDist = FLT_MAX;
        for (const CGameArea::Dock& dock : area->GetDocks())
        {
            zeus::CVector3f dockCenter = (dock.GetPlaneVertices()[0] + dock.GetPlaneVertices()[1] +
                                          dock.GetPlaneVertices()[2] + dock.GetPlaneVertices()[3]) * 0.25f;
            dockCenter.z = 0.f;
            float dist = (playerXYPos - dockCenter).magSquared();
            if (dist < closestDist)
            {
                TAreaId connAreaId = dock.GetConnectedAreaId(0);
                if (connAreaId != kInvalidAreaId)
                {
                    std::unique_ptr<CGameArea>& connArea = x850_world->GetGameAreas()[x8cc_nextAreaId];
                    if (connArea->IsPostConstructed())
                    {
                        CGameArea::EOcclusionState occState = connArea->GetPostConstructed()->x10dc_occlusionState;
                        if (occState == CGameArea::EOcclusionState::Occluded)
                        {
                            closestDist = dist;
                            lastArea = connArea.get();
                        }
                    }
                }
            }
        }

        if (lastArea != nullptr)
        {
            if (closestDist != 0.f)
                closestDist /= std::sqrt(closestDist);
            closestDist -= 2.f;
            if (closestDist < 8.f)
            {
                if (closestDist > 0.f)
                    closestDist = (closestDist / 8.f) * 0.5f + 0.5f;
                else
                    closestDist = 0.5f;

                xf24_thermColdScale1 =
                    (1.f - closestDist) * lastArea->GetPostConstructed()->x111c_thermalCurrent +
                                closestDist * area->GetPostConstructed()->x111c_thermalCurrent;
                return;
            }
        }

        xf24_thermColdScale1 = area->GetPostConstructed()->x111c_thermalCurrent;
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

void CStateManager::SendScriptMsg(CEntity* dest, TUniqueId src, EScriptObjectMessage msg)
{
    if (dest && !dest->x30_26_scriptingBlocked)
    {
        dest->AcceptScriptMsg(msg, src, *this);
    }
}

void CStateManager::SendScriptMsg(TUniqueId dest, TUniqueId src, EScriptObjectMessage msg)
{
    CEntity* ent = ObjectById(dest);
    SendScriptMsg(ent, src, msg);
}

void CStateManager::SendScriptMsg(TUniqueId src, TEditorId dest,
                                  EScriptObjectMessage msg, EScriptObjectState state)
{
    CEntity* ent = ObjectById(src);
    auto search = GetIdListForScript(dest);
    if (ent &&
        search.first != x890_scriptIdMap.cend() &&
        search.second != x890_scriptIdMap.cend())
    {
        for (auto it = search.first ; it != search.second ; ++it)
        {
            TUniqueId id = it->second;
            CEntity* dobj = x80c_allObjs->GetObjectById(id);
            SendScriptMsg(dobj, src, msg);
        }
    }
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

std::pair<std::multimap<TEditorId, TUniqueId>::const_iterator,
          std::multimap<TEditorId, TUniqueId>::const_iterator>
CStateManager::GetIdListForScript(TEditorId id) const
{
    return x890_scriptIdMap.equal_range(id);
}

void CStateManager::LoadScriptObjects(TAreaId, CInputStream& in, std::vector<TEditorId>& idsOut)
{
}

void CStateManager::LoadScriptObject(TAreaId, EScriptObjectType, u32,
                                     CInputStream& in, EScriptPersistence)
{
}

void CStateManager::InitScriptObjects(std::vector<TEditorId>& ids)
{
    for (TEditorId id : ids)
    {
        if (id == kInvalidEditorId)
            continue;
        TUniqueId uid = GetIdForScript(id);
        SendScriptMsg(uid, kInvalidUniqueId, EScriptObjectMessage::InternalMessage13);
    }
    MurderScriptInstanceNames();
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

void CStateManager::InitializeState(ResId mlvlId, TAreaId aid, ResId mreaId)
{
    bool hadRandom = x900_activeRandom != nullptr;
    x900_activeRandom = &x8fc_random;

    if (xb3c_initPhase == InitPhase::LoadWorld)
    {
        CreateStandardGameObjects();
        x850_world.reset(new CWorld(*g_SimplePool, *g_ResFactory, mlvlId));
        xb3c_initPhase = InitPhase::LoadFirstArea;
    }

    if (xb3c_initPhase == InitPhase::LoadFirstArea)
    {
        if (!x8f0_shadowTex.IsLoaded())
            return;
        x8f0_shadowTex.GetObj();

        if (!x850_world->CheckWorldComplete(this, aid, mreaId))
            return;
        x8cc_nextAreaId = x850_world->x68_curAreaId;
        CGameArea* area = x850_world->x18_areas[x8cc_nextAreaId].get();
        if (x850_world->ScheduleAreaToLoad(area, *this))
        {
            area->StartStreamIn(*this);
            return;
        }
        xb3c_initPhase = InitPhase::Done;
    }

    SetCurrentAreaId(x8cc_nextAreaId);
    g_GameState->CurrentWorldState().SetAreaId(x8cc_nextAreaId);
    x850_world->TravelToArea(x8cc_nextAreaId, *this, true);
    UpdateRoomAcoustics(x8cc_nextAreaId);

    TUniqueId entId = x80c_allObjs->GetFirstObjectIndex();
    while (entId != kInvalidUniqueId)
    {
        CEntity* ent = x80c_allObjs->GetObjectById(entId);
        SendScriptMsg(ent, kInvalidUniqueId, EScriptObjectMessage::InternalMessage14);
        entId = x80c_allObjs->GetNextObjectIndex(entId);
    }

    entId = x80c_allObjs->GetFirstObjectIndex();
    while (entId != kInvalidUniqueId)
    {
        CScriptSpawnPoint* sp = dynamic_cast<CScriptSpawnPoint*>(x80c_allObjs->GetObjectById(entId));
        if (sp && sp->x30_24_active && sp->FirstSpawn())
        {
            const zeus::CTransform& xf = sp->GetTransform();
            zeus::CVector3f lookVec{xf.basis[0][1], xf.basis[1][1], xf.basis[2][1]};
            if (lookVec.canBeNormalized())
            {
                auto lookXf = zeus::lookAt(xf.origin, lookVec);
                x84c_player->Teleport(lookXf, *this, true);
            }

            if (!g_GameState->x228_25_deferPowerupInit)
                break;

            g_GameState->x228_25_deferPowerupInit = false;
            for (int i=0 ; i<41 ; ++i)
            {
                CPlayerState::EItemType iType = CPlayerState::EItemType(i);

                u32 spawnPu = sp->GetPowerup(iType);
                u32 statePu = x8b8_playerState->GetItemCapacity(iType);
                if (statePu < spawnPu)
                    x8b8_playerState->InitializePowerUp(iType, spawnPu - statePu);

                spawnPu = sp->GetPowerup(iType);
                statePu = x8b8_playerState->GetItemAmount(iType);
                if (statePu < spawnPu)
                    x8b8_playerState->IncrPickup(iType, spawnPu - statePu);
            }
        }
        entId = x80c_allObjs->GetNextObjectIndex(entId);
    }

    x84c_player->AsyncLoadSuit(*this);
    x870_cameraManager->ResetCameras(*this);

    if (!hadRandom)
        x900_activeRandom = nullptr;
    else
        x900_activeRandom = &x8fc_random;

    x880_envFxManager->AsyncLoadResources(*this);
}

void CStateManager::CreateStandardGameObjects()
{
    float height = g_tweakPlayer->GetPlayerHeight();
    float xyHe = g_tweakPlayer->GetPlayerXYHalfExtent();
    float unk1 = g_tweakPlayer->GetPlayerSomething1();
    float unk2 = g_tweakPlayer->GetPlayerSomething2();
    float unk3 = g_tweakPlayer->GetPlayerSomething3();
    zeus::CAABox pBounds = {{-xyHe, -xyHe, 0.f}, {xyHe, xyHe, height}};
    auto q = zeus::CQuaternion::fromAxisAngle(zeus::CVector3f{0.f, 0.f, 1.f}, zeus::degToRad(129.6f));
    x84c_player.reset(new CPlayer(AllocateUniqueId(), zeus::CTransform(q), pBounds, 0,
                                  zeus::CVector3f{1.65f, 1.65f, 1.65f},
                                  200.f, unk1, unk2, unk3, CMaterialList(EMaterialTypes::ThirtyTwo,
                                  EMaterialTypes::Nineteen, EMaterialTypes::ThirtySeven)));
    AddObject(*x84c_player);
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

void CStateManager::SetCurrentAreaId(TAreaId aid)
{
    if (aid != x8cc_nextAreaId)
    {
        x8d0_prevAreaId = x8cc_nextAreaId;
        UpdateRoomAcoustics(aid);
        x8cc_nextAreaId = aid;
    }

    if (aid == kInvalidAreaId)
        return;
    if (x8c0_mapWorldInfo->IsAreaVisted(aid))
        return;
    x8c0_mapWorldInfo->SetAreaVisited(aid, true);
    x850_world->GetMapWorld()->RecalculateWorldSphere(*x8c0_mapWorldInfo, *x850_world);
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

void CStateManager::AddObject(CEntity&)
{
}

void CStateManager::AddObject(CEntity*)
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
