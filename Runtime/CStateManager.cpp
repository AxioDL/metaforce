#include "CStateManager.hpp"
#include "Camera/CCameraShakeData.hpp"
#include "Camera/CGameCamera.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "World/CGameLight.hpp"
#include "CSortedLists.hpp"
#include "Weapon/CWeaponMgr.hpp"
#include "CFluidPlaneManager.hpp"
#include "World/CEnvFxManager.hpp"
#include "World/CActorModelParticles.hpp"
#include "World/CTeamAiTypes.hpp"
#include "World/CScriptPlayerActor.hpp"
#include "Input/CRumbleManager.hpp"
#include "World/CWorld.hpp"
#include "Graphics/CLight.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CPlayerState.hpp"
#include "CGameState.hpp"
#include "World/CPlayer.hpp"
#include "Weapon/CPlayerGun.hpp"
#include "World/CMorphBall.hpp"
#include "World/CScriptSpawnPoint.hpp"
#include "AutoMapper/CMapWorldInfo.hpp"
#include "Particle/CGenDescription.hpp"
#include "CMemoryCardSys.hpp"
#include "TCastTo.hpp"
#include "World/CScriptSpecialFunction.hpp"
#include "CTimeProvider.hpp"
#include "Camera/CBallCamera.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "World/CScriptDock.hpp"
#include "Particle/CDecalManager.hpp"

#include <cmath>

namespace urde
{
logvisor::Module LogModule("urde::CStateManager");
CStateManager::CStateManager(const std::weak_ptr<CRelayTracker>& relayTracker,
                             const std::weak_ptr<CMapWorldInfo>& mwInfo, const std::weak_ptr<CPlayerState>& playerState,
                             const std::weak_ptr<CWorldTransManager>& wtMgr,
                             const std::weak_ptr<CWorldLayerState>& layerState)
: x80c_allObjs(new CObjectList(EGameObjectList::All))
, x814_actorObjs(new CActorList())
, x81c_physActorObjs(new CPhysicsActorList())
, x824_cameraObjs(new CGameCameraList())
, x82c_lightObjs(new CGameLightList())
, x834_listenAiObjs(new CListeningAiList())
, x83c_aiWaypointObjs(new CAiWaypointList())
, x844_platformAndDoorObjs(new CPlatformAndDoorList())
, x8b8_playerState(playerState)
, x8bc_relayTracker(relayTracker)
, x8c0_mapWorldInfo(mwInfo)
, x8c4_worldTransManager(wtMgr)
, x8c8_worldLayerState(layerState)
{
    x86c_stateManagerContainer.reset(new CStateManagerContainer);
    x870_cameraManager = &x86c_stateManagerContainer->x0_cameraManager;
    x874_sortedListManager = &x86c_stateManagerContainer->x3c0_sortedListManager;
    x878_weaponManager = &x86c_stateManagerContainer->xe3d8_weaponManager;
    x87c_fluidPlaneManager = &x86c_stateManagerContainer->xe3ec_fluidPlaneManager;
    x880_envFxManager = &x86c_stateManagerContainer->xe510_envFxManager;
    x884_actorModelParticles = &x86c_stateManagerContainer->xf168_actorModelParticles;
    x88c_rumbleManager = &x86c_stateManagerContainer->xf250_rumbleManager;

    g_Renderer->SetDrawableCallback(&CStateManager::RendererDrawCallback, this);

    x90c_loaderFuncs[int(EScriptObjectType::Actor)] = ScriptLoader::LoadActor;
    x90c_loaderFuncs[int(EScriptObjectType::Waypoint)] = ScriptLoader::LoadWaypoint;
    x90c_loaderFuncs[int(EScriptObjectType::Door)] = ScriptLoader::LoadDoor;
    x90c_loaderFuncs[int(EScriptObjectType::Trigger)] = ScriptLoader::LoadTrigger;
    x90c_loaderFuncs[int(EScriptObjectType::Timer)] = ScriptLoader::LoadTimer;
    x90c_loaderFuncs[int(EScriptObjectType::Counter)] = ScriptLoader::LoadCounter;
    x90c_loaderFuncs[int(EScriptObjectType::Effect)] = ScriptLoader::LoadEffect;
    x90c_loaderFuncs[int(EScriptObjectType::Platform)] = ScriptLoader::LoadPlatform;
    x90c_loaderFuncs[int(EScriptObjectType::Sound)] = ScriptLoader::LoadSound;
    x90c_loaderFuncs[int(EScriptObjectType::Generator)] = ScriptLoader::LoadGenerator;
    x90c_loaderFuncs[int(EScriptObjectType::Dock)] = ScriptLoader::LoadDock;
    x90c_loaderFuncs[int(EScriptObjectType::Camera)] = ScriptLoader::LoadCamera;
    x90c_loaderFuncs[int(EScriptObjectType::CameraWaypoint)] = ScriptLoader::LoadCameraWaypoint;
    x90c_loaderFuncs[int(EScriptObjectType::NewIntroBoss)] = ScriptLoader::LoadNewIntroBoss;
    x90c_loaderFuncs[int(EScriptObjectType::SpawnPoint)] = ScriptLoader::LoadSpawnPoint;
    x90c_loaderFuncs[int(EScriptObjectType::CameraHint)] = ScriptLoader::LoadCameraHint;
    x90c_loaderFuncs[int(EScriptObjectType::Pickup)] = ScriptLoader::LoadPickup;
    x90c_loaderFuncs[int(EScriptObjectType::MemoryRelay)] = ScriptLoader::LoadMemoryRelay;
    x90c_loaderFuncs[int(EScriptObjectType::RandomRelay)] = ScriptLoader::LoadRandomRelay;
    x90c_loaderFuncs[int(EScriptObjectType::Relay)] = ScriptLoader::LoadRelay;
    x90c_loaderFuncs[int(EScriptObjectType::Beetle)] = ScriptLoader::LoadBeetle;
    x90c_loaderFuncs[int(EScriptObjectType::HUDMemo)] = ScriptLoader::LoadHUDMemo;
    x90c_loaderFuncs[int(EScriptObjectType::CameraFilterKeyframe)] = ScriptLoader::LoadCameraFilterKeyframe;
    x90c_loaderFuncs[int(EScriptObjectType::CameraBlurKeyframe)] = ScriptLoader::LoadCameraBlurKeyframe;
    x90c_loaderFuncs[int(EScriptObjectType::DamageableTrigger)] = ScriptLoader::LoadDamageableTrigger;
    x90c_loaderFuncs[int(EScriptObjectType::Debris)] = ScriptLoader::LoadDebris;
    x90c_loaderFuncs[int(EScriptObjectType::CameraShaker)] = ScriptLoader::LoadCameraShaker;
    x90c_loaderFuncs[int(EScriptObjectType::ActorKeyframe)] = ScriptLoader::LoadActorKeyframe;
    x90c_loaderFuncs[int(EScriptObjectType::Water)] = ScriptLoader::LoadWater;
    x90c_loaderFuncs[int(EScriptObjectType::Warwasp)] = ScriptLoader::LoadWarWasp;
    x90c_loaderFuncs[int(EScriptObjectType::SpacePirate)] = ScriptLoader::LoadSpacePirate;
    x90c_loaderFuncs[int(EScriptObjectType::FlyingPirate)] = ScriptLoader::LoadFlyingPirate;
    x90c_loaderFuncs[int(EScriptObjectType::ElitePirate)] = ScriptLoader::LoadElitePirate;
    x90c_loaderFuncs[int(EScriptObjectType::MetroidBeta)] = ScriptLoader::LoadMetroidBeta;
    x90c_loaderFuncs[int(EScriptObjectType::ChozoGhost)] = ScriptLoader::LoadChozoGhost;
    x90c_loaderFuncs[int(EScriptObjectType::CoverPoint)] = ScriptLoader::LoadCoverPoint;
    x90c_loaderFuncs[int(EScriptObjectType::SpiderBallWaypoint)] = ScriptLoader::LoadSpiderBallWaypoint;
    x90c_loaderFuncs[int(EScriptObjectType::BloodFlower)] = ScriptLoader::LoadBloodFlower;
    x90c_loaderFuncs[int(EScriptObjectType::FlickerBat)] = ScriptLoader::LoadFlickerBat;
    x90c_loaderFuncs[int(EScriptObjectType::PathCamera)] = ScriptLoader::LoadPathCamera;
    x90c_loaderFuncs[int(EScriptObjectType::GrapplePoint)] = ScriptLoader::LoadGrapplePoint;
    x90c_loaderFuncs[int(EScriptObjectType::PuddleSpore)] = ScriptLoader::LoadPuddleSpore;
    x90c_loaderFuncs[int(EScriptObjectType::DebugCameraWaypoint)] = ScriptLoader::LoadDebugCameraWaypoint;
    x90c_loaderFuncs[int(EScriptObjectType::SpiderBallAttractionSurface)] =
        ScriptLoader::LoadSpiderBallAttractionSurface;
    x90c_loaderFuncs[int(EScriptObjectType::PuddleToadGamma)] = ScriptLoader::LoadPuddleToadGamma;
    x90c_loaderFuncs[int(EScriptObjectType::DistanceFog)] = ScriptLoader::LoadDistanceFog;
    x90c_loaderFuncs[int(EScriptObjectType::FireFlea)] = ScriptLoader::LoadFireFlea;
    x90c_loaderFuncs[int(EScriptObjectType::MetareeAlpha)] = ScriptLoader::LoadMetareeAlpha;
    x90c_loaderFuncs[int(EScriptObjectType::DockAreaChange)] = ScriptLoader::LoadDockAreaChange;
    x90c_loaderFuncs[int(EScriptObjectType::ActorRotate)] = ScriptLoader::LoadActorRotate;
    x90c_loaderFuncs[int(EScriptObjectType::SpecialFunction)] = ScriptLoader::LoadSpecialFunction;
    x90c_loaderFuncs[int(EScriptObjectType::SpankWeed)] = ScriptLoader::LoadSpankWeed;
    x90c_loaderFuncs[int(EScriptObjectType::Parasite)] = ScriptLoader::LoadParasite;
    x90c_loaderFuncs[int(EScriptObjectType::PlayerHint)] = ScriptLoader::LoadPlayerHint;
    x90c_loaderFuncs[int(EScriptObjectType::Ripper)] = ScriptLoader::LoadRipper;
    x90c_loaderFuncs[int(EScriptObjectType::PickupGenerator)] = ScriptLoader::LoadPickupGenerator;
    x90c_loaderFuncs[int(EScriptObjectType::AIKeyframe)] = ScriptLoader::LoadAIKeyframe;
    x90c_loaderFuncs[int(EScriptObjectType::PointOfInterest)] = ScriptLoader::LoadPointOfInterest;
    x90c_loaderFuncs[int(EScriptObjectType::Drone)] = ScriptLoader::LoadDrone;
    x90c_loaderFuncs[int(EScriptObjectType::MetroidAlpha)] = ScriptLoader::LoadMetroidAlpha;
    x90c_loaderFuncs[int(EScriptObjectType::DebrisExtended)] = ScriptLoader::LoadDebrisExtended;
    x90c_loaderFuncs[int(EScriptObjectType::Steam)] = ScriptLoader::LoadSteam;
    x90c_loaderFuncs[int(EScriptObjectType::Ripple)] = ScriptLoader::LoadRipple;
    x90c_loaderFuncs[int(EScriptObjectType::BallTrigger)] = ScriptLoader::LoadBallTrigger;
    x90c_loaderFuncs[int(EScriptObjectType::TargetingPoint)] = ScriptLoader::LoadTargetingPoint;
    x90c_loaderFuncs[int(EScriptObjectType::EMPulse)] = ScriptLoader::LoadEMPulse;
    x90c_loaderFuncs[int(EScriptObjectType::IceSheegoth)] = ScriptLoader::LoadIceSheegoth;
    x90c_loaderFuncs[int(EScriptObjectType::PlayerActor)] = ScriptLoader::LoadPlayerActor;
    x90c_loaderFuncs[int(EScriptObjectType::Flaahgra)] = ScriptLoader::LoadFlaahgra;
    x90c_loaderFuncs[int(EScriptObjectType::AreaAttributes)] = ScriptLoader::LoadAreaAttributes;
    x90c_loaderFuncs[int(EScriptObjectType::FishCloud)] = ScriptLoader::LoadFishCloud;
    x90c_loaderFuncs[int(EScriptObjectType::FishCloudModifier)] = ScriptLoader::LoadFishCloudModifier;
    x90c_loaderFuncs[int(EScriptObjectType::VisorFlare)] = ScriptLoader::LoadVisorFlare;
    x90c_loaderFuncs[int(EScriptObjectType::WorldTeleporter)] = ScriptLoader::LoadWorldTeleporter;
    x90c_loaderFuncs[int(EScriptObjectType::VisorGoo)] = ScriptLoader::LoadVisorGoo;
    x90c_loaderFuncs[int(EScriptObjectType::JellyZap)] = ScriptLoader::LoadJellyZap;
    x90c_loaderFuncs[int(EScriptObjectType::ControllerAction)] = ScriptLoader::LoadControllerAction;
    x90c_loaderFuncs[int(EScriptObjectType::Switch)] = ScriptLoader::LoadSwitch;
    x90c_loaderFuncs[int(EScriptObjectType::PlayerStateChange)] = ScriptLoader::LoadPlayerStateChange;
    x90c_loaderFuncs[int(EScriptObjectType::Thardus)] = ScriptLoader::LoadThardus;
    x90c_loaderFuncs[int(EScriptObjectType::WallCrawlerSwarm)] = ScriptLoader::LoadWallCrawlerSwarm;
    x90c_loaderFuncs[int(EScriptObjectType::AIJumpPoint)] = ScriptLoader::LoadAiJumpPoint;
    x90c_loaderFuncs[int(EScriptObjectType::FlaahgraTentacle)] = ScriptLoader::LoadFlaahgraTentacle;
    x90c_loaderFuncs[int(EScriptObjectType::RoomAcoustics)] = ScriptLoader::LoadRoomAcoustics;
    x90c_loaderFuncs[int(EScriptObjectType::ColorModulate)] = ScriptLoader::LoadColorModulate;
    x90c_loaderFuncs[int(EScriptObjectType::ThardusRockProjectile)] = ScriptLoader::LoadThardusRockProjectile;
    x90c_loaderFuncs[int(EScriptObjectType::Midi)] = ScriptLoader::LoadMidi;
    x90c_loaderFuncs[int(EScriptObjectType::StreamedAudio)] = ScriptLoader::LoadStreamedAudio;
    x90c_loaderFuncs[int(EScriptObjectType::WorldTeleporterToo)] = ScriptLoader::LoadWorldTeleporter;
    x90c_loaderFuncs[int(EScriptObjectType::Repulsor)] = ScriptLoader::LoadRepulsor;
    x90c_loaderFuncs[int(EScriptObjectType::GunTurret)] = ScriptLoader::LoadGunTurret;
    x90c_loaderFuncs[int(EScriptObjectType::FogVolume)] = ScriptLoader::LoadFogVolume;
    x90c_loaderFuncs[int(EScriptObjectType::Babygoth)] = ScriptLoader::LoadBabygoth;
    x90c_loaderFuncs[int(EScriptObjectType::Eyeball)] = ScriptLoader::LoadEyeball;
    x90c_loaderFuncs[int(EScriptObjectType::RadialDamage)] = ScriptLoader::LoadRadialDamage;
    x90c_loaderFuncs[int(EScriptObjectType::CameraPitchVolume)] = ScriptLoader::LoadCameraPitchVolume;
    x90c_loaderFuncs[int(EScriptObjectType::EnvFxDensityController)] = ScriptLoader::LoadEnvFxDensityController;
    x90c_loaderFuncs[int(EScriptObjectType::Magdolite)] = ScriptLoader::LoadMagdolite;
    x90c_loaderFuncs[int(EScriptObjectType::TeamAIMgr)] = ScriptLoader::LoadTeamAIMgr;
    x90c_loaderFuncs[int(EScriptObjectType::SnakeWeedSwarm)] = ScriptLoader::LoadSnakeWeedSwarm;
    x90c_loaderFuncs[int(EScriptObjectType::ActorContraption)] = ScriptLoader::LoadActorContraption;
    x90c_loaderFuncs[int(EScriptObjectType::Oculus)] = ScriptLoader::LoadOculus;
    x90c_loaderFuncs[int(EScriptObjectType::Geemer)] = ScriptLoader::LoadGeemer;
    x90c_loaderFuncs[int(EScriptObjectType::SpindleCamera)] = ScriptLoader::LoadSpindleCamera;
    x90c_loaderFuncs[int(EScriptObjectType::AtomicAlpha)] = ScriptLoader::LoadAtomicAlpha;
    x90c_loaderFuncs[int(EScriptObjectType::CameraHintTrigger)] = ScriptLoader::LoadCameraHintTrigger;
    x90c_loaderFuncs[int(EScriptObjectType::RumbleEffect)] = ScriptLoader::LoadRumbleEffect;
    x90c_loaderFuncs[int(EScriptObjectType::AmbientAI)] = ScriptLoader::LoadAmbientAI;
    x90c_loaderFuncs[int(EScriptObjectType::AtomicBeta)] = ScriptLoader::LoadAtomicBeta;
    x90c_loaderFuncs[int(EScriptObjectType::IceZoomer)] = ScriptLoader::LoadIceZoomer;
    x90c_loaderFuncs[int(EScriptObjectType::Puffer)] = ScriptLoader::LoadPuffer;
    x90c_loaderFuncs[int(EScriptObjectType::Tryclops)] = ScriptLoader::LoadTryclops;
    x90c_loaderFuncs[int(EScriptObjectType::Ridley)] = ScriptLoader::LoadRidley;
    x90c_loaderFuncs[int(EScriptObjectType::Seedling)] = ScriptLoader::LoadSeedling;
    x90c_loaderFuncs[int(EScriptObjectType::ThermalHeatFader)] = ScriptLoader::LoadThermalHeatFader;
    x90c_loaderFuncs[int(EScriptObjectType::Burrower)] = ScriptLoader::LoadBurrower;
    x90c_loaderFuncs[int(EScriptObjectType::ScriptBeam)] = ScriptLoader::LoadBeam;
    x90c_loaderFuncs[int(EScriptObjectType::WorldLightFader)] = ScriptLoader::LoadWorldLightFader;
    x90c_loaderFuncs[int(EScriptObjectType::MetroidPrimeStage2)] = ScriptLoader::LoadMetroidPrimeStage2;
    x90c_loaderFuncs[int(EScriptObjectType::MetroidPrimeStage1)] = ScriptLoader::LoadMetroidPrimeStage1;
    x90c_loaderFuncs[int(EScriptObjectType::MazeNode)] = ScriptLoader::LoadMazeNode;
    x90c_loaderFuncs[int(EScriptObjectType::OmegaPirate)] = ScriptLoader::LoadOmegaPirate;
    x90c_loaderFuncs[int(EScriptObjectType::PhazonPool)] = ScriptLoader::LoadPhazonPool;
    x90c_loaderFuncs[int(EScriptObjectType::PhazonHealingNodule)] = ScriptLoader::LoadPhazonHealingNodule;
    x90c_loaderFuncs[int(EScriptObjectType::NewCameraShaker)] = ScriptLoader::LoadNewCameraShaker;
    x90c_loaderFuncs[int(EScriptObjectType::ShadowProjector)] = ScriptLoader::LoadShadowProjector;
    x90c_loaderFuncs[int(EScriptObjectType::EnergyBall)] = ScriptLoader::LoadEnergyBall;

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
                                          dock.GetPlaneVertices()[2] + dock.GetPlaneVertices()[3]) *
                                         0.25f;
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

                xf24_thermColdScale1 = (1.f - closestDist) * lastArea->GetPostConstructed()->x111c_thermalCurrent +
                                       closestDist * area->GetPostConstructed()->x111c_thermalCurrent;
                return;
            }
        }

        xf24_thermColdScale1 = area->GetPostConstructed()->x111c_thermalCurrent;
    }
}

void CStateManager::RendererDrawCallback(const void* drawable, const void* ctx, int type)
{
    CStateManager& mgr = reinterpret_cast<CStateManager&>(ctx);
    switch (type)
    {
    case 0:
    {
        CActor& actor = reinterpret_cast<CActor&>(drawable);
        if (actor.xc8_drawnToken == mgr.x8dc_objectDrawToken)
            break;
        if (actor.xc6_nextDrawNode != kInvalidUniqueId)
            mgr.RecursiveDrawTree(actor.xc6_nextDrawNode);
        actor.Render(mgr);
        actor.xc8_drawnToken = mgr.x8dc_objectDrawToken;
        break;
    }
    case 1:
        reinterpret_cast<CSimpleShadow&>(drawable).Render(mgr.x8f0_shadowTex.GetObj());
        break;
    case 2:
        reinterpret_cast<CDecal&>(drawable).Render();
        break;
    default: break;
    }
}

bool CStateManager::RenderLast(TUniqueId) { return false; }

void CStateManager::AddDrawableActorPlane(const CActor& actor, const zeus::CPlane& plane,
                                          const zeus::CAABox& aabb) const
{
    const_cast<CActor&>(actor).SetAddedToken(x8dc_objectDrawToken + 1);
    g_Renderer->AddPlaneObject(static_cast<const void*>(&actor), aabb, plane, 0);
}

void CStateManager::AddDrawableActor(const CActor& actor, const zeus::CVector3f& vec, const zeus::CAABox& aabb) const
{
    const_cast<CActor&>(actor).SetAddedToken(x8dc_objectDrawToken + 1);
    g_Renderer->AddDrawable(static_cast<const void*>(&actor), vec, aabb, 0,
                            IRenderer::EDrawableSorting::SortedCallback);
}

bool CStateManager::SpecialSkipCinematic()
{
    if (xf38_skipCineSpecialFunc == kInvalidUniqueId)
        return false;

    CScriptSpecialFunction* ent = static_cast<CScriptSpecialFunction*>(ObjectById(xf38_skipCineSpecialFunc));
    if (!ent || !ent->ShouldSkipCinematic(*this))
        return false;

    bool hadRandom = x900_activeRandom != nullptr;
    SetActiveRandomToDefault();
    x870_cameraManager->SkipCinematic(*this);
    ent->SkipCinematic(*this);
    x900_activeRandom = hadRandom ? &x8fc_random : nullptr;

    return true;
}

TAreaId CStateManager::GetVisAreaId() const
{
    const CGameCamera* cam = static_cast<const CGameCamera*>(x870_cameraManager->GetCurrentCamera(*this));
    const CBallCamera* ballCam = x870_cameraManager->GetBallCamera();
    TAreaId curArea = x850_world->x68_curAreaId;
    if (cam != ballCam)
        return curArea;

    const zeus::CVector3f& camTranslation = ballCam->GetTranslation();
    zeus::CAABox camAABB(camTranslation, camTranslation);
    camAABB.accumulateBounds(x84c_player->GetTranslation());
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    BuildNearList(nearList, camAABB, CMaterialFilter(EMaterialTypes::AIBlock, CMaterialList(),
                                                     CMaterialFilter::EFilterType::One), nullptr);
    for (TUniqueId id : nearList)
        if (TCastToConstPtr<CScriptDock> dock = GetObjectById(id))
            if (dock->GetDestinationAreaId() == curArea && dock->HasPointCrossedDock(*this, camTranslation))
                return dock->GetCurrentConnectedAreaId(*this);

    return curArea;
}

void CStateManager::GetWeaponIdCount(TUniqueId, EWeaponType) {}

void CStateManager::RemoveWeaponId(TUniqueId, EWeaponType) {}

void CStateManager::AddWeaponId(TUniqueId, EWeaponType) {}

void CStateManager::UpdateEscapeSequenceTimer(float) {}

float CStateManager::GetEscapeSequenceTimer() const { return 0.f; }

void CStateManager::ResetEscapeSequenceTimer(float) {}

void CStateManager::SetupParticleHook(const CActor& actor) const {}

void CStateManager::MurderScriptInstanceNames() {}

std::string CStateManager::HashInstanceName(CInputStream& in)
{
#ifdef NDEBUG
    static std::string name;
    while (in.readByte() != 0) {};
    return name;
#else
    return in.readString();
#endif
}

void CStateManager::SetActorAreaId(CActor& actor, TAreaId) {}

void CStateManager::TouchSky() const
{
    x850_world->TouchSky();
}

void CStateManager::TouchPlayerActor()
{
    if (xf6c_playerActor == kInvalidUniqueId)
        return;

#if 0
    TCastToPtr<CScriptPlayerActor> spa(ObjectById(xf6c_playerActor));
    if (spa)
        spa->TouchModels();
#endif
}

void CStateManager::DrawSpaceWarp(const zeus::CVector3f& v, float strength) const
{
    CPlayerState::EPlayerVisor visor = x8b8_playerState->GetActiveVisor(*this);
    if (visor == CPlayerState::EPlayerVisor::Scan || visor == CPlayerState::EPlayerVisor::Combat)
    {
        zeus::CVector3f screenV = TCastToConstPtr<CGameCamera>(x870_cameraManager->GetCurrentCamera(*this))->ConvertToScreenSpace(v);
        g_Renderer->DrawSpaceWarp(screenV, strength);
    }
}

void CStateManager::DrawReflection(const zeus::CVector3f&) {}

void CStateManager::CacheReflection() {}

bool CStateManager::CanCreateProjectile(TUniqueId, EWeaponType, int) const { return false; }

const CGameLightList* CStateManager::GetDynamicLightList() const { return nullptr; }

void CStateManager::BuildDynamicLightListForWorld()
{
    if (x8b8_playerState->GetActiveVisor(*this) == CPlayerState::EPlayerVisor::Thermal)
    {
        x8e0_dynamicLights.clear();
        return;
    }

    if (x82c_lightObjs->size() == 0)
        return;

    x8e0_dynamicLights.clear();
    for (CEntity* ent : *x82c_lightObjs)
    {
        CGameLight& light = static_cast<CGameLight&>(*ent);
        if (light.GetActive())
        {
            CLight l = light.GetLight();
            if (l.GetIntensity() > FLT_EPSILON && l.GetRadius() > FLT_EPSILON)
                x8e0_dynamicLights.push_back(l);
        }
    }
}

void CStateManager::DrawDebugStuff() const {}

void CStateManager::RenderCamerasAndAreaLights() const {}

void CStateManager::DrawE3DeathEffect() const {}

void CStateManager::DrawAdditionalFilters() const {}

zeus::CFrustum CStateManager::SetupViewForDraw(const SViewport& vp) const
{
    const CGameCamera* cam = static_cast<const CGameCamera*>(x870_cameraManager->GetCurrentCamera(*this));
    zeus::CTransform camXf = x870_cameraManager->GetCurrentCameraTransform(*this);
    g_Renderer->SetWorldViewpoint(camXf);
    CBooModel::SetNewPlayerPositionAndTime(x84c_player->GetTranslation());
    int vpWidth = xf2c_viewportScale.x * vp.x8_width;
    int vpHeight = xf2c_viewportScale.y * vp.xc_height;
    int vpLeft = (vp.x8_width - vpWidth) / 2 + vp.x0_left;
    int vpTop = (vp.xc_height - vpHeight) / 2 + vp.x4_top;
    g_Renderer->SetViewport(vpLeft, vpTop, vpWidth, vpHeight);
    CGraphics::SetDepthRange(0.125f, 1.f);
    float fov = std::atan(std::tan(zeus::degToRad(cam->GetFov()) * 0.5f) * xf2c_viewportScale.y * 2.f);
    float width = xf2c_viewportScale.x * vp.x8_width;
    float height = xf2c_viewportScale.y * vp.xc_height;
    g_Renderer->SetPerspective(zeus::radToDeg(fov), width, height,
                               cam->GetNearClipDistance(), cam->GetFarClipDistance());
    zeus::CFrustum frustum;
    zeus::CProjection proj;
    proj.setPersp(zeus::SProjPersp{fov, width / height, cam->GetNearClipDistance(), cam->GetFarClipDistance()});
    frustum.updatePlanes(camXf, proj);
    g_Renderer->SetClippingPlanes(frustum);
    //g_Renderer->PrimColor(zeus::CColor::skWhite);
    CGraphics::SetModelMatrix(zeus::CTransform::Identity());
    x87c_fluidPlaneManager->StartFrame(false);
    g_Renderer->SetDebugOption(IRenderer::EDebugOption::One, 1);
    return frustum;
}

void CStateManager::DrawWorld() const
{
    CTimeProvider timeProvider(xf14_);
    zeus::CFrustum frustum = SetupViewForDraw(g_Viewport);

    /* Area camera is in (not necessarily player) */
    TAreaId visAreaId = GetVisAreaId();

    x850_world->TouchSky();

    int areaCount = 0;
    CGameArea* areaArr[10];
    for (CGameArea* area = x850_world->GetChainHead(EChain::Alive);
         area != CWorld::AliveAreasEnd() && areaCount != 10;
         area = area->x130_next)
    {
        CGameArea::EOcclusionState occState = CGameArea::EOcclusionState::NotOccluded;
        if (area->IsPostConstructed())
            occState = area->GetOcclusionState();
        if (occState == CGameArea::EOcclusionState::Occluded)
            areaArr[areaCount++] = area;
    }

    std::sort(std::begin(areaArr), std::begin(areaArr) + areaCount,
    [visAreaId](CGameArea* a, CGameArea* b) -> bool
    {
        if (a->x4_selfIdx == b->x4_selfIdx)
            return false;
        if (visAreaId == a->x4_selfIdx)
            return false;
        if (visAreaId == b->x4_selfIdx)
            return true;
        return CGraphics::g_ViewPoint.dot(a->GetAABB().center()) >
               CGraphics::g_ViewPoint.dot(b->GetAABB().center());
    });

    int pvsCount = 0;
    CPVSVisSet pvsArr[10];
    for (CGameArea** area = areaArr;
         area != areaArr + areaCount;
         ++area)
    {
        CGameArea* areaPtr = *area;
        CPVSVisSet& pvsSet = pvsArr[pvsCount++];
        pvsSet.Reset(EPVSVisSetState::OutOfBounds);
        GetVisSetForArea(areaPtr->x4_selfIdx, visAreaId, pvsSet);
    }

    int mask;
    int targetMask;
    CPlayerState::EPlayerVisor visor = x8b8_playerState->GetActiveVisor(*this);
    bool thermal = visor == CPlayerState::EPlayerVisor::Thermal;
    if (thermal)
    {
        const_cast<CStateManager&>(*this).xf34_particleFlags = 1;
        mask = 52;
        targetMask = 0;
    }
    else
    {
        const_cast<CStateManager&>(*this).xf34_particleFlags = 2;
        mask = 1 << (visor == CPlayerState::EPlayerVisor::XRay ? 3 : 1);
        targetMask = 0;
    }

    g_Renderer->SetThermal(thermal, g_tweakGui->GetThermalVisorLevel(), g_tweakGui->GetThermalVisorColor());
    g_Renderer->SetThermalColdScale(xf28_thermColdScale2 + xf24_thermColdScale1);

    for (int i=areaCount-1 ; i>=0 ; --i)
    {
        CGameArea& area = *areaArr[i];
        SetupFogForArea(area);
        g_Renderer->EnablePVS(&pvsArr[i], area.x4_selfIdx);
        g_Renderer->SetWorldLightFadeLevel(area.GetPostConstructed()->x1128_worldLightingLevel);
        g_Renderer->DrawUnsortedGeometry(area.x4_selfIdx, mask, targetMask);
    }

    if (!SetupFogForDraw())
        g_Renderer->SetWorldFog(ERglFogMode::None, 0.f, 1.f, zeus::CColor::skBlack);

    x850_world->DrawSky(zeus::CTransform::Translate(CGraphics::g_ViewPoint));

    if (areaCount)
        SetupFogForArea(*areaArr[areaCount-1]);

    for (TUniqueId id : x86c_stateManagerContainer->xf370_)
    {
        if (const CActor* ent = static_cast<const CActor*>(GetObjectById(id)))
        {
            if (!thermal || ent->xe6_27_ & 0x2)
            {
                ent->Render(*this);
            }
        }
    }

    bool morphingPlayerVisible = false;
    int thermalActorCount = 0;
    CActor* thermalActorArr[1024];
    for (int i=0 ; i<areaCount ; ++i)
    {
        CGameArea& area = *areaArr[i];
        CPVSVisSet& pvs = pvsArr[i];
        bool isVisArea = area.x4_selfIdx == visAreaId;
        SetupFogForArea(area);
        g_Renderer->SetWorldLightFadeLevel(area.GetPostConstructed()->x1128_worldLightingLevel);
        for (CEntity* ent : *area.GetPostConstructed()->x10c0_areaObjs)
        {
            if (TCastToPtr<CActor> actor = ent)
            {
                if (!actor->xe7_29_)
                    continue;
                TUniqueId actorId = actor->GetUniqueId();
                if (!thermal && area.LookupPVSUniqueID(actorId) == actorId)
                    if (pvs.GetVisible(area.LookupPVSID(actorId)) == EPVSVisSetState::EndOfTree)
                        continue;
                if (x84c_player.get() == actor.GetPtr())
                {
                    if (thermal)
                        continue;
                    switch (x84c_player->GetMorphballTransitionState())
                    {
                    case CPlayer::EPlayerMorphBallState::Unmorphed:
                    case CPlayer::EPlayerMorphBallState::Morphed:
                        x84c_player->AddToRenderer(frustum, *this);
                        continue;
                    default:
                        morphingPlayerVisible = true;
                        continue;
                    }
                }
                if (!thermal || actor->xe6_27_ & 0x2)
                    actor->AddToRenderer(frustum, *this);
                if (thermal && actor->xe6_27_ & 0x4)
                    thermalActorArr[thermalActorCount++] = actor.GetPtr();
            }
        }

        if (isVisArea && !thermal)
        {
            CDecalManager::AddToRenderer(frustum, *this);
            x884_actorModelParticles->AddStragglersToRenderer(*this);
        }

        ++const_cast<CStateManager&>(*this).x8dc_objectDrawToken;

        // TODO: Finish
        x84c_player->GetMorphBall();
    }
}

void CStateManager::SetupFogForArea(const CGameArea& area) const
{
    if (SetupFogForDraw())
        return;

    if (x8b8_playerState->GetActiveVisor(*this) == CPlayerState::EPlayerVisor::XRay)
    {
        float fogDist = area.GetXRayFogDistance();
        float farz = g_tweakGui->GetXRayFogNearZ() * (1.f - fogDist) +
                     g_tweakGui->GetXRayFogFarZ() * fogDist;
        g_Renderer->SetWorldFog(ERglFogMode(g_tweakGui->GetXRayFogMode()),
                                g_tweakGui->GetXRayFogNearZ(),
                                farz, g_tweakGui->GetXRayFogColor());
    }
    else
    {
        area.GetAreaFog()->SetCurrent();
    }
}

bool CStateManager::SetupFogForDraw() const
{
    switch (x8b8_playerState->GetActiveVisor(*this))
    {
    case CPlayerState::EPlayerVisor::Thermal:
        g_Renderer->SetWorldFog(ERglFogMode::None, 0.f, 1.f, zeus::CColor::skBlack);
        return true;
    case CPlayerState::EPlayerVisor::XRay:
    default:
        return false;
    case CPlayerState::EPlayerVisor::Combat:
    case CPlayerState::EPlayerVisor::Scan:
        auto& fog = x870_cameraManager->Fog();
        if (fog.IsFogDisabled())
            return false;
        fog.SetCurrent();
        return true;
    }
}

void CStateManager::PreRender()
{
    if (xf94_24_)
    {
        x86c_stateManagerContainer->xf370_.clear();
        x86c_stateManagerContainer->xf39c_.clear();
        xf7c_ = 0;
        x850_world->PreRender();
        BuildDynamicLightListForWorld();
        CGameCamera* cam = static_cast<CGameCamera*>(x870_cameraManager->GetCurrentCamera(*this));
        zeus::CFrustum frustum;
        zeus::CProjection proj;
        proj.setPersp(zeus::SProjPersp{zeus::degToRad(cam->GetFov()),
                                       cam->GetAspectRatio(), cam->GetNearClipDistance(), cam->GetFarClipDistance()});
        frustum.updatePlanes(x870_cameraManager->GetCurrentCameraTransform(*this), proj);
        for (CGameArea* area = x850_world->GetChainHead(EChain::Alive);
             area != CWorld::AliveAreasEnd();
             area = area->x130_next)
        {
            CGameArea::EOcclusionState occState = CGameArea::EOcclusionState::NotOccluded;
            if (area->IsPostConstructed())
                occState = area->GetOcclusionState();
            if (occState == CGameArea::EOcclusionState::Occluded)
            {
                for (CEntity* ent : *area->GetPostConstructed()->x10c0_areaObjs)
                {
                    if (TCastToPtr<CActor> act = ent)
                    {
                        if (act->GetE7_29())
                        {
                            act->CalculateRenderBounds();
                            act->PreRender(*this, frustum);
                        }
                    }
                }
            }
        }

        CacheReflection();
        g_Renderer->PrepareDynamicLights(x8e0_dynamicLights);
    }
}

bool CStateManager::GetVisSetForArea(TAreaId a, TAreaId b, CPVSVisSet& setOut) const
{
    if (b == kInvalidAreaId)
        return false;

    zeus::CVector3f viewPoint = CGraphics::g_ViewMatrix.origin;
    zeus::CVector3f closestDockPoint = viewPoint;
    bool hasClosestDock = false;
    if (a != b)
    {
        CGameArea& area = *x850_world->GetGameAreas()[b];
        if (area.IsPostConstructed())
        {
            for (const CGameArea::Dock& dock : area.GetDocks())
            {
                for (int i=0 ; i<dock.GetDockRefs().size() ; ++i)
                {
                    TAreaId connArea = dock.GetConnectedAreaId(i);
                    if (connArea == a)
                    {
                        const auto& verts = dock.GetPlaneVertices();
                        zeus::CVector3f dockCenter = (verts[0] + verts[1] + verts[2] + verts[4]) * 0.25f;
                        if (hasClosestDock)
                            if ((dockCenter - viewPoint).magSquared() >=
                                (closestDockPoint - viewPoint).magSquared())
                                continue;
                        closestDockPoint = dockCenter;
                        hasClosestDock = true;
                    }
                }
            }
        }
    }
    else
    {
        hasClosestDock = true;
    }

    if (hasClosestDock)
    {
        if (CPVSAreaSet* pvs = x850_world->GetGameAreas()[a]->GetPostConstructed()->xa0_pvs.get())
        {
            const CPVSVisOctree& octree = pvs->GetVisOctree();
            zeus::CVector3f closestDockLocal =
                x850_world->GetGameAreas()[a]->GetInverseTransform() * closestDockPoint;
            CPVSVisSet set;
            set.SetTestPoint(octree, closestDockLocal);

            if (set.GetState() == EPVSVisSetState::NodeFound)
            {
                setOut = set;
                return true;
            }
        }
    }

    return false;
}

void CStateManager::RecursiveDrawTree(TUniqueId node) const
{
    if (TCastToConstPtr<CActor> actor = GetObjectById(node))
    {
        if (x8dc_objectDrawToken != actor->xc8_drawnToken)
        {
            if (actor->xc6_nextDrawNode != kInvalidUniqueId)
                RecursiveDrawTree(actor->xc6_nextDrawNode);
            if (x8dc_objectDrawToken == actor->xcc_addedToken)
                actor->Render(*this);
            const_cast<CActor*>(actor.GetPtr())->xc8_drawnToken = x8dc_objectDrawToken;
        }
    }
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

void CStateManager::SendScriptMsgAlways(TUniqueId dest, TUniqueId src, EScriptObjectMessage msg)
{
    CEntity* dst = ObjectById(dest);
    if (dst)
        dst->AcceptScriptMsg(msg, src, *this);
}

void CStateManager::SendScriptMsg(TUniqueId src, TEditorId dest, EScriptObjectMessage msg, EScriptObjectState state)
{
    CEntity* ent = ObjectById(src);
    auto search = GetIdListForScript(dest);
    if (ent && search.first != x890_scriptIdMap.cend() && search.second != x890_scriptIdMap.cend())
    {
        for (auto it = search.first; it != search.second; ++it)
        {
            TUniqueId id = it->second;
            CEntity* dobj = x80c_allObjs->GetObjectById(id);
            SendScriptMsg(dobj, src, msg);
        }
    }
}

void CStateManager::FreeScriptObjects(TAreaId aid)
{
    for (const auto& p : x890_scriptIdMap)
        if (p.first.AreaNum() == aid)
            FreeScriptObject(p.second);

    for (auto it = x8a4_loadedScriptObjects.begin() ; it != x8a4_loadedScriptObjects.end() ;)
    {
        if (it->first.AreaNum() == aid)
        {
            it = x8a4_loadedScriptObjects.erase(it);
            continue;
        }
        ++it;
    }

    CGameArea* area = x850_world->GetGameAreas()[aid].get();
    if (area->IsPostConstructed())
    {
        const CGameArea::CPostConstructed* pc = area->GetPostConstructed();
        for (CEntity* ent : *pc->x10c0_areaObjs)
            if (ent && !ent->IsInUse())
                FreeScriptObject(ent->GetUniqueId());
    }
}

void CStateManager::FreeScriptObject(TUniqueId id)
{
    CEntity* ent = ObjectById(id);
    if (!ent || ent->IsInGraveyard())
        return;

    ent->SetIsInGraveyard(true);
    x858_objectGraveyard.push_back(id);
    ent->AcceptScriptMsg(EScriptObjectMessage::Deleted, kInvalidUniqueId, *this);
    ent->SetIsScriptingBlocked(true);

    if (TCastToPtr<CActor> act = ent)
    {
        x874_sortedListManager->Remove(act.GetPtr());
        act->SetUseInSortedLists(false);
    }
}

std::pair<const SScriptObjectStream*, TEditorId> CStateManager::GetBuildForScript(TEditorId id) const
{
    auto search = x8a4_loadedScriptObjects.find(id);
    if (search == x8a4_loadedScriptObjects.cend())
        return {nullptr, kInvalidEditorId};
    return {&search->second, search->first};
}

TEditorId CStateManager::GetEditorIdForUniqueId(TUniqueId id) const
{
    const CEntity* ent = GetObjectById(id);
    if (ent)
        return ent->GetEditorId();
    return kInvalidEditorId;
}

TUniqueId CStateManager::GetIdForScript(TEditorId id) const
{
    auto search = x890_scriptIdMap.find(id);
    if (search == x890_scriptIdMap.cend())
        return kInvalidUniqueId;
    return search->second;
}

std::pair<std::multimap<TEditorId, TUniqueId>::const_iterator, std::multimap<TEditorId, TUniqueId>::const_iterator>
CStateManager::GetIdListForScript(TEditorId id) const
{
    return x890_scriptIdMap.equal_range(id);
}

void CStateManager::LoadScriptObjects(TAreaId aid, CInputStream& in, std::vector<TEditorId>& idsOut)
{
    in.readUByte();
    int objCount = in.readUint32Big();
    idsOut.reserve(idsOut.size() + objCount);
    for (int i=0 ; i<objCount ; ++i)
    {
        EScriptObjectType objType = EScriptObjectType(in.readUByte());
        u32 objSize = in.readUint32Big();
        u32 pos = in.position();
        auto id = LoadScriptObject(aid, objType, objSize, in);
        if (id.first == kInvalidEditorId)
            continue;
        auto build = GetBuildForScript(id.first);
        if (build.first)
            continue;
        x8a4_loadedScriptObjects[id.first] = SScriptObjectStream{objType, pos, objSize};
        idsOut.push_back(id.first);
    }
}

std::pair<TEditorId, TUniqueId> CStateManager::LoadScriptObject(TAreaId aid, EScriptObjectType type,
                                                                u32 length, CInputStream& in)
{
    TEditorId id = in.readUint32Big();
    u32 connCount = in.readUint32Big();
    length -= 8;
    std::vector<SConnection> conns;
    conns.reserve(connCount);
    for (int i=0 ; i<connCount ; ++i)
    {
        EScriptObjectState state = EScriptObjectState(in.readUint32Big());
        EScriptObjectMessage msg = EScriptObjectMessage(in.readUint32Big());
        TEditorId target = in.readUint32Big();
        length -= 12;
        conns.push_back(SConnection{state, msg, target});
    }
    u32 propCount = in.readUint32Big();
    length -= 4;
    auto startPos = in.position();

    bool error = false;
    FScriptLoader loader = {};
    if (type < EScriptObjectType::ScriptObjectTypeMAX && type >= EScriptObjectType::Actor)
        loader = x90c_loaderFuncs[int(type)];

    CEntity* ent = nullptr;
    if (loader)
    {
        CEntityInfo info(aid, conns, id);
        ent = loader(*this, in, propCount, info);
    }
    else
    {
        error = true;
    }

    if (ent)
        AddObject(ent);
    else
        error = true;

    u32 readAmt = in.position() - startPos;
    if (readAmt > length)
        LogModule.report(logvisor::Fatal, "Script object overread");
    u32 leftover = length - readAmt;
    for (u32 i=0 ; i<leftover ; ++i)
        in.readByte();

    if (error || ent == nullptr)
    {
        LogModule.report(logvisor::Fatal, "Script load error");
        return {kInvalidEditorId, kInvalidUniqueId};
    }
    else
        return {id, ent->GetUniqueId()};
}

std::pair<TEditorId, TUniqueId> CStateManager::GenerateObject(TEditorId)
{
    return {kInvalidEditorId, kInvalidUniqueId};
}

void CStateManager::InitScriptObjects(const std::vector<TEditorId>& ids)
{
    for (TEditorId id : ids)
    {
        if (id == kInvalidEditorId)
            continue;
        TUniqueId uid = GetIdForScript(id);
        SendScriptMsg(uid, kInvalidUniqueId, EScriptObjectMessage::Constructed);
    }
    MurderScriptInstanceNames();
}

void CStateManager::InformListeners(const zeus::CVector3f&, EListenNoiseType) {}

bool CStateManager::ApplyKnockBack(CActor& actor, const CDamageInfo& info, const CDamageVulnerability&,
                                   const zeus::CVector3f&, float)
{
    return false;
}

bool CStateManager::ApplyDamageToWorld(TUniqueId, const CActor&, const zeus::CVector3f&, const CDamageInfo& info,
                                       const CMaterialFilter&)
{
    return false;
}

void CStateManager::ProcessRadiusDamage(const CActor&, CActor&, const zeus::CVector3f&, const CDamageInfo& info,
                                        const CMaterialFilter&)
{
}

bool CStateManager::ApplyRadiusDamage(const CActor&, const zeus::CVector3f&, CActor&, const CDamageInfo& info)
{
    return false;
}

bool CStateManager::ApplyLocalDamage(const zeus::CVector3f& vec1, const zeus::CVector3f& vec2, CActor& actor, float dt,
                                     const CWeaponMode& weapMode)
{
    CHealthInfo* hInfo = actor.HealthInfo();
    if (!hInfo || dt < 0.f)
        return false;

    if (hInfo->GetHP() <= 0.f)
        return true;

    float f30 = dt;

    CPlayer* player = TCastToPtr<CPlayer>(actor);
    CAi* ai = TCastToPtr<CAi>(actor);
#if 0
    CDestroyableRock* dRock = nullptr;
    if (!ai)
        TCastToPtr<CDestroyableRock>(actor);
#endif

    if (player)
    {
        if (x870_cameraManager->IsInCinematicCamera())
        {
        }
    }
    return false;
}

bool CStateManager::ApplyDamage(TUniqueId, TUniqueId, TUniqueId, const CDamageInfo& info, const CMaterialFilter&)
{
    return false;
}

void CStateManager::UpdateAreaSounds() {}

void CStateManager::FrameEnd() {}

void CStateManager::ProcessPlayerInput() {}

void CStateManager::ProcessInput(const CFinalInput& input) {}

void CStateManager::Update(float dt) {}

void CStateManager::UpdateGameState() {}

void CStateManager::FrameBegin(s32 frameCount) {}

void CStateManager::InitializeState(ResId mlvlId, TAreaId aid, ResId mreaId)
{
    bool hadRandom = x900_activeRandom != nullptr;
    SetActiveRandomToDefault();

    if (xb3c_initPhase == EInitPhase::LoadWorld)
    {
        CreateStandardGameObjects();
        x850_world.reset(new CWorld(*g_SimplePool, *g_ResFactory, mlvlId));
        xb3c_initPhase = EInitPhase::LoadFirstArea;
    }

    if (xb3c_initPhase == EInitPhase::LoadFirstArea)
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
        xb3c_initPhase = EInitPhase::Done;
    }

    SetCurrentAreaId(x8cc_nextAreaId);
    g_GameState->CurrentWorldState().SetAreaId(x8cc_nextAreaId);
    x850_world->TravelToArea(x8cc_nextAreaId, *this, true);
    UpdateRoomAcoustics(x8cc_nextAreaId);

    for (CEntity* ent : *x80c_allObjs)
        SendScriptMsg(ent, kInvalidUniqueId, EScriptObjectMessage::InternalMessage14);

    for (CEntity* ent : *x80c_allObjs)
    {
        CScriptSpawnPoint* sp = TCastToPtr<CScriptSpawnPoint>(ent);
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
            for (int i = 0; i < int(CPlayerState::EItemType::Max); ++i)
            {
                CPlayerState::EItemType iType = CPlayerState::EItemType(i);

                u32 spawnPu = sp->GetPowerup(iType);
                u32 statePu = x8b8_playerState->GetItemAmount(iType);
                if (statePu < spawnPu)
                    x8b8_playerState->InitializePowerUp(iType, spawnPu - statePu);

                spawnPu = sp->GetPowerup(iType);
                statePu = x8b8_playerState->GetItemAmount(iType);
                if (statePu < spawnPu)
                    x8b8_playerState->IncrPickup(iType, spawnPu - statePu);
            }
        }
    }

    x84c_player->AsyncLoadSuit(*this);
    x870_cameraManager->ResetCameras(*this);

    if (!hadRandom)
        ClearActiveRandom();
    else
        SetActiveRandomToDefault();

    x880_envFxManager->AsyncLoadResources(*this);
}

void CStateManager::CreateStandardGameObjects()
{
    float height = g_tweakPlayer->GetPlayerHeight();
    float xyHe = g_tweakPlayer->GetPlayerXYHalfExtent();
    float unk1 = g_tweakPlayer->GetX274();
    float unk2 = g_tweakPlayer->GetX278();
    float unk3 = g_tweakPlayer->GetX27C();
    zeus::CAABox pBounds = {{-xyHe, -xyHe, 0.f}, {xyHe, xyHe, height}};
    auto q = zeus::CQuaternion::fromAxisAngle(zeus::CVector3f{0.f, 0.f, 1.f}, zeus::degToRad(129.6f));
    x84c_player.reset(new CPlayer(
        AllocateUniqueId(), zeus::CTransform(q), pBounds,
        g_tweakPlayerRes->xc4_ballTransitionsANCS,
        zeus::CVector3f{1.65f, 1.65f, 1.65f}, 200.f, unk1, unk2,
        unk3, CMaterialList(EMaterialTypes::Player,
        EMaterialTypes::Solid, EMaterialTypes::GroundCollider)));
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

void CStateManager::RemoveObject(TUniqueId) {}

void CStateManager::RemoveActor(TUniqueId) {}

void CStateManager::UpdateRoomAcoustics(TAreaId) {}

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

void CStateManager::ClearGraveyard() {}

void CStateManager::DeleteObjectRequest(TUniqueId id)
{
    CEntity* entity = ObjectById(id);
    if (!entity)
        return;

    if (entity->IsInGraveyard())
        return;

    entity->SetIsInGraveyard(true);

    x858_objectGraveyard.push_back(entity->GetUniqueId());
    entity->AcceptScriptMsg(EScriptObjectMessage::Deleted, kInvalidUniqueId, *this);
    entity->SetIsScriptingBlocked(true);

    if (TCastToPtr<CActor> actor = entity)
    {
        x874_sortedListManager->Remove(actor);
        actor->SetUseInSortedLists(false);
    }
}

CEntity* CStateManager::ObjectById(TUniqueId uid) { return x80c_allObjs->GetObjectById(uid); }
const CEntity* CStateManager::GetObjectById(TUniqueId uid) const { return x80c_allObjs->GetObjectById(uid); }

void CStateManager::AreaUnloaded(TAreaId) {}

void CStateManager::PrepareAreaUnload(TAreaId) {}

void CStateManager::AreaLoaded(TAreaId) {}

void CStateManager::BuildNearList(rstl::reserved_vector<TUniqueId, 1024>& listOut, const zeus::CVector3f&,
                                  const zeus::CVector3f&, float, const CMaterialFilter&, const CActor*) const
{
}

void CStateManager::BuildColliderList(rstl::reserved_vector<TUniqueId, 1024>& listOut, const CActor&,
                                      const zeus::CAABox&) const
{
}

void CStateManager::BuildNearList(rstl::reserved_vector<TUniqueId, 1024>& listOut, const zeus::CAABox&,
                                  const CMaterialFilter&, const CActor*) const
{
}

void CStateManager::UpdateActorInSortedLists(CActor&) {}

void CStateManager::UpdateSortedLists() {}

zeus::CAABox CStateManager::CalculateObjectBounds(const CActor&) { return {}; }

void CStateManager::AddObject(CEntity&) {}

void CStateManager::AddObject(CEntity*) {}

bool CStateManager::RayStaticIntersection(const zeus::CVector3f&, const zeus::CVector3f&, float,
                                          const CMaterialFilter&) const
{
    return false;
}

bool CStateManager::RayWorldIntersection(TUniqueId, const zeus::CVector3f&, const zeus::CVector3f&, float,
                                         const CMaterialFilter&,
                                         const rstl::reserved_vector<TUniqueId, 1024>& list) const
{
    return false;
}

void CStateManager::UpdateObjectInLists(CEntity&) {}

TUniqueId CStateManager::AllocateUniqueId()
{
    const s16 lastIndex = x0_nextFreeIndex;
    s16 ourIndex;
    do
    {
        ourIndex = x0_nextFreeIndex;
        x0_nextFreeIndex = (x0_nextFreeIndex + 1) & 0x3ff;
        if (x0_nextFreeIndex == lastIndex)
            LogModule.report(logvisor::Fatal, "Object List Full!");
    }
    while (x80c_allObjs->GetObjectByIndex(ourIndex) != nullptr);

    x8_idArr[ourIndex] = (x8_idArr[ourIndex] + 1) & 0x3f;
    if (((ourIndex | ((x8_idArr[ourIndex]) << 10)) & 0xFFFF) == kInvalidUniqueId)
        x8_idArr[0] = 0;

    return ((ourIndex | ((x8_idArr[ourIndex]) << 10)) & 0xFFFF);
}

void CStateManager::DeferStateTransition(EStateManagerTransition t)
{
    if (t == EStateManagerTransition::InGame)
    {
        if (xf90_deferredTransition != EStateManagerTransition::InGame)
        {
            x850_world->SetPauseState(false);
            xf90_deferredTransition = EStateManagerTransition::InGame;
        }
    }
    else
    {
        if (xf90_deferredTransition == EStateManagerTransition::InGame)
        {
            x850_world->SetPauseState(true);
            xf90_deferredTransition = t;
        }
    }
}

bool CStateManager::CanShowMapScreen() const
{
    const CHintOptions::SHintState* curDispHint = g_GameState->HintOptions().GetCurrentDisplayedHint();
    if (!curDispHint || curDispHint->CanContinue())
        return true;
    return false;
}

std::pair<u32, u32> CStateManager::CalculateScanCompletionRate() const
{
    u32 num = 0;
    u32 denom = 0;
    int idx = 0;
    for (const std::pair<u32, float>& scan : x8b8_playerState->GetScanTimes())
    {
        CSaveWorld::EScanCategory category = g_MemoryCardSys->GetScanStates()[idx++].second;
        if (category != CSaveWorld::EScanCategory::None && category != CSaveWorld::EScanCategory::Research)
        {
            ++denom;
            if (scan.second == 1.f)
                ++num;
        }
    }
    return {num, denom};
}

bool CStateManager::ApplyDamage(TUniqueId, TUniqueId, TUniqueId, const CDamageInfo& info, const CMaterialFilter&,
                                const zeus::CVector3f&)
{
    return false;
}
}
