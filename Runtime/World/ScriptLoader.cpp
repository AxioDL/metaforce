#include "ScriptLoader.hpp"
#include "logvisor/logvisor.hpp"

namespace urde
{
static logvisor::Module Log("urde::ScriptLoader");

static bool EnsurePropertyCount(int count, int expected, const char* structName)
{
    if (count < expected)
    {
        Log.report(logvisor::Fatal, "Insufficient number of props (%d/%d) for %s entity",
                   count, expected, structName);
        return false;
    }
    return true;
}

CEntity* ScriptLoader::LoadActor(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 24, "Actor"))
        return nullptr;


}

CEntity* ScriptLoader::LoadWaypoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadDoorArea(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadTrigger(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadTimer(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadCounter(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadEffect(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPlatform(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSound(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadGenerator(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadDock(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadCamera(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadCameraWaypoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadNewIntroBoss(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSpawnPoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadCameraHint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPickup(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadMemoryRelay(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadRandomRelay(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadRelay(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadBeetle(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadHUDMemo(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadCameraFilterKeyframe(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadCameraBlurKeyframe(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadDamageableTrigger(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadDebris(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadCameraShaker(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadActorKeyframe(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadWater(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadWarwasp(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSpacePirate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadFlyingPirate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadElitePirate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadMetroidBeta(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadChozoGhost(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadCoverPoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSpiderBallWaypoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadBloodFlower(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadFlickerBat(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPathCamera(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadGrapplePoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPuddleSpore(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadDebugCameraWaypoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSpiderBallAttractionSurface(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPuddleToadGamma(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadDistanceFog(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadFireFlea(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadMetareeAlpha(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadDockAreaChange(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadActorRotate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSpecialFunction(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSpankWeed(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadParasite(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPlayerHint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadRipper(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPickupGenerator(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadAIKeyframe(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPointOfInterest(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadDrone(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadMetroidAlpha(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadDebrisExtended(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSteam(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadRipple(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadBallTrigger(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadTargetingPoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadElectroMagneticPulse(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadIceSheegoth(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPlayerActor(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadFlaahgra(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadAreaAttributes(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadFishCloud(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadFishCloudModifier(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadVisorFlare(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadWorldTeleporter(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadVisorGoo(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadJellyZap(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadControllerAction(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSwitch(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPlayerStateChange(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadThardus(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadWallCrawlerSwarm(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadAIJumpPoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadFlaahgraTentacle(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadRoomAcoustics(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadColorModulate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadThardusRockProjectile(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadMidi(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadStreamedAudio(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadRepulsor(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadGunTurret(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadFogVolume(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadBabygoth(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadEyeball(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadRadialDamage(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadCameraPitchVolume(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadEnvFxDensityController(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadMagdolite(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadTeamAIMgr(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSnakeWeedSwarm(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::Load(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadActorContraption(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadOculus(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadGeemer(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSpindleCamera(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadAtomicAlpha(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadCameraHintTrigger(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadRumbleEffect(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadAmbientAI(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadAtomicBeta(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadIceZoomer(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPuffer(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadTryclops(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadRidley(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSeedling(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadThermalHeatFader(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadBurrower(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadScriptBeam(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadWorldLightFader(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadMetroidPrimeStage2(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadMetroidPrimeStage1(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadMazeNode(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadOmegaPirate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPhazonPool(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPhazonHealingNodule(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadNewCameraShaker(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadShadowProjector(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadEnergyBall(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
}

u32 ScriptLoader::LoadParameterFlags(CInputStream& in)
{
    u32 count = in.readUint32Big();
    u32 ret = 0;
    for (u32 i=0 ; i<count ; ++i)
        if (in.readBool())
            ret |= 1 << i;
    return ret;
}

}
