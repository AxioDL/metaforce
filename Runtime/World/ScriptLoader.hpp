#ifndef __URDE_SCRIPTLOADER_HPP__
#define __URDE_SCRIPTLOADER_HPP__

#include "IOStreams.hpp"
#include "zeus/CTransform.hpp"

namespace urde
{
class CStateManager;
class CEntityInfo;
class CEntity;
class CGrappleParameters;
class CActorParameters;
class CVisorParameters;
class CScannableParameters;
class CLightParameters;
class CAnimationParameters;
class CFluidUVMotion;
class CCameraShakeData;

using FScriptLoader = std::function<CEntity*(CStateManager& mgr, CInputStream& in,
                                             int propCount, const CEntityInfo& info)>;

class ScriptLoader
{
public:
    static u32 LoadParameterFlags(CInputStream& in);
    static CGrappleParameters LoadGrappleParameters(CInputStream& in);
    static CActorParameters LoadActorParameters(CInputStream& in);
    static CVisorParameters LoadVisorParameters(CInputStream& in);
    static CScannableParameters LoadScannableParameters(CInputStream& in);
    static CLightParameters LoadLightParameters(CInputStream& in);
    static CAnimationParameters LoadAnimationParameters(CInputStream& in);
    static CFluidUVMotion LoadFluidUVMotion(CInputStream& in);
    static CCameraShakeData LoadCameraShakeData(CInputStream& in);
    static zeus::CTransform ConvertEditorEulerToTransform4f(const zeus::CVector3f& orientation,
                                                            const zeus::CVector3f& position);

    static CEntity* LoadActor(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadWaypoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadDoor(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadTrigger(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadTimer(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadCounter(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadEffect(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadPlatform(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadSound(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadGenerator(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadDock(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadCamera(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadCameraWaypoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadNewIntroBoss(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadSpawnPoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadCameraHint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadPickup(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadMemoryRelay(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadRandomRelay(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadRelay(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadBeetle(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadHUDMemo(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadCameraFilterKeyframe(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadCameraBlurKeyframe(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadDamageableTrigger(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadDebris(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadCameraShaker(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadActorKeyframe(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadWater(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadWarWasp(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadSpacePirate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadFlyingPirate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadElitePirate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadMetroidBeta(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadChozoGhost(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadCoverPoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadSpiderBallWaypoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadBloodFlower(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadFlickerBat(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadPathCamera(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadGrapplePoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadPuddleSpore(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadDebugCameraWaypoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadSpiderBallAttractionSurface(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadPuddleToadGamma(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadDistanceFog(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadFireFlea(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadMetareeAlpha(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadDockAreaChange(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadActorRotate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadSpecialFunction(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadSpankWeed(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadParasite(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadPlayerHint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadRipper(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadPickupGenerator(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadAIKeyframe(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadPointOfInterest(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadDrone(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadMetroidAlpha(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadDebrisExtended(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadSteam(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadRipple(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadBallTrigger(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadTargetingPoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadEMPulse(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadIceSheegoth(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadPlayerActor(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadFlaahgra(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadAreaAttributes(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadFishCloud(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadFishCloudModifier(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadVisorFlare(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadWorldTeleporter(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadVisorGoo(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadJellyZap(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadControllerAction(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadSwitch(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadPlayerStateChange(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadThardus(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadWallCrawlerSwarm(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadAiJumpPoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadFlaahgraTentacle(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadRoomAcoustics(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadColorModulate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadThardusRockProjectile(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadMidi(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadStreamedAudio(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadRepulsor(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadGunTurret(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadFogVolume(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadBabygoth(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadEyeball(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadRadialDamage(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadCameraPitchVolume(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadEnvFxDensityController(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadMagdolite(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadTeamAIMgr(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadSnakeWeedSwarm(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* Load(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadActorContraption(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadOculus(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadGeemer(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadSpindleCamera(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadAtomicAlpha(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadCameraHintTrigger(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadRumbleEffect(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadAmbientAI(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadAtomicBeta(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadIceZoomer(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadPuffer(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadTryclops(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadRidley(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadSeedling(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadThermalHeatFader(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadBurrower(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadBeam(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadWorldLightFader(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadMetroidPrimeStage2(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadMetroidPrimeStage1(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadMazeNode(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadOmegaPirate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadPhazonPool(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadPhazonHealingNodule(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadNewCameraShaker(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadShadowProjector(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
    static CEntity* LoadEnergyBall(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info);
};

}

#endif // __URDE_SCRIPTLOADER_HPP__
