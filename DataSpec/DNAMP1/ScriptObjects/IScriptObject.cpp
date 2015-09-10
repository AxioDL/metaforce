#include "IScriptObject.hpp"
#include "ScriptTypes.hpp"

namespace Retro
{
namespace DNAMP1
{
namespace priv
{
static ScriptObjectSpec ActorEnt = {0x00, []() -> IScriptObject* {return new struct Actor();}};
static ScriptObjectSpec WaypointEnt = {0x02, []() -> IScriptObject* {return new struct Waypoint();}};
static ScriptObjectSpec DoorAreaEnt = {0x03, []() -> IScriptObject* {return new struct DoorArea();}};
static ScriptObjectSpec TriggerEnt = {0x04, []() -> IScriptObject* {return new struct Trigger(); }};
static ScriptObjectSpec TimerEnt = {0x05, []() -> IScriptObject* {return new struct Timer(); }};
static ScriptObjectSpec CounterEnt = {0x06, []() -> IScriptObject* {return new struct Counter(); }};
static ScriptObjectSpec EffectEnt = {0x07, []() -> IScriptObject* {return new struct Effect(); }};
static ScriptObjectSpec PlatformEnt = {0x08, []() -> IScriptObject* {return new struct Platform(); }};
static ScriptObjectSpec SoundEnt = {0x09, []() -> IScriptObject* {return new struct Sound(); }};
static ScriptObjectSpec GeneratorEnt = {0x0A, []() -> IScriptObject* {return new struct Generator();}};
static ScriptObjectSpec DockEnt = {0x0B, []() -> IScriptObject* {return new struct Dock(); }};
static ScriptObjectSpec CameraEnt = {0x0C, []() -> IScriptObject* {return new struct Camera(); }};
static ScriptObjectSpec CameraWaypointEnt = {0x0D, []() -> IScriptObject* {return new struct CameraWaypoint(); }};
static ScriptObjectSpec NewIntroBossEnt = {0x0E, []() -> IScriptObject* {return new struct NewIntroBoss(); }};
static ScriptObjectSpec SpawnPointEnt = {0x0F, []() -> IScriptObject* {return new struct SpawnPoint(); }};
static ScriptObjectSpec CameraHintEnt = {0x10, []() -> IScriptObject* {return new struct CameraHint(); }};
static ScriptObjectSpec PickupEnt = {0x11, []() -> IScriptObject* {return new struct Pickup(); }};
static ScriptObjectSpec MemoryRelayEnt = {0x13, []() -> IScriptObject* {return new struct MemoryRelay(); }};
static ScriptObjectSpec RandomRelayEnt = {0x14, []() -> IScriptObject* {return new struct RandomRelay(); }};
static ScriptObjectSpec RelayEnt = {0x15, []() -> IScriptObject* {return new struct Relay(); }};
static ScriptObjectSpec BeetleEnt = {0x16, []() -> IScriptObject* {return new struct Beetle(); }};
static ScriptObjectSpec HUDMemoEnt = {0x17, []() -> IScriptObject* {return new struct HUDMemo(); }};
static ScriptObjectSpec CameraFilterKeyframeEnt = {0x18, []() -> IScriptObject* {return new struct CameraFilterKeyframe(); }};
static ScriptObjectSpec CameraBlurKeyframeEnt = {0x19, []() -> IScriptObject* {return new struct CameraBlurKeyframe(); }};
static ScriptObjectSpec DamageableTriggerEnt = {0x1A, []() -> IScriptObject* {return new struct DamageableTrigger(); }};
static ScriptObjectSpec DebrisEnt = {0x1B, []() -> IScriptObject* {return new struct Debris(); }};
static ScriptObjectSpec CameraShakerEnt = {0x1C, []() -> IScriptObject* {return new struct CameraShaker(); }};
static ScriptObjectSpec ActorKeyframeEnt = {0x1D, []() -> IScriptObject* {return new struct ActorKeyframe(); }};
static ScriptObjectSpec WaterEnt = {0x20, []() -> IScriptObject* {return new struct Water(); }};
static ScriptObjectSpec WarwaspEnt = {0x21, []() -> IScriptObject* {return new struct Warwasp(); }};
static ScriptObjectSpec SpacePirateEnt = {0x24, []() -> IScriptObject* {return new struct SpacePirate(); }};
static ScriptObjectSpec FlyingPirateEnt = {0x25, []() -> IScriptObject* {return new struct FlyingPirate(); }};
static ScriptObjectSpec ElitePirateEnt = {0x26, []() -> IScriptObject* {return new struct ElitePirate(); }};
static ScriptObjectSpec MetroidBetaEnt = {0x27, []() -> IScriptObject* {return new struct MetroidBeta(); }};
static ScriptObjectSpec ChozoGhostEnt = {0x28, []() -> IScriptObject* {return new struct ChozoGhost(); }};
static ScriptObjectSpec CoverPointEnt = {0x2A, []() -> IScriptObject* {return new struct CoverPoint(); }};
static ScriptObjectSpec SpiderBallWaypointEnt = {0x2C, []() -> IScriptObject* {return new struct SpiderBallWaypoint(); }};
static ScriptObjectSpec BloodFlowerEnt = {0x2D, []() -> IScriptObject* {return new struct BloodFlower(); }};
static ScriptObjectSpec FlickerBatEnt = {0x2E, []() -> IScriptObject* {return new struct FlickerBat(); }};
static ScriptObjectSpec PathCameraEnt = {0x2F, []() -> IScriptObject* {return new struct PathCamera(); }};
static ScriptObjectSpec GrapplePointEnt = {0x30, []() -> IScriptObject* {return new struct GrapplePoint(); }};
static ScriptObjectSpec PuddleSporeEnt = {0x31, []() -> IScriptObject* {return new struct PuddleSpore(); }};
static ScriptObjectSpec DebugCameraWaypointEnt = {0x32, []() -> IScriptObject* {return new struct DebugCameraWaypoint(); }};
static ScriptObjectSpec SpiderBallAttractionSurfaceEnt = {0x33, []() -> IScriptObject* {return new struct SpiderBallAttractionSurface(); }};
static ScriptObjectSpec PuddleToadGammaEnt = {0x34, []() -> IScriptObject* {return new struct PuddleToadGamma(); }};
static ScriptObjectSpec DistanceFogEnt = {0x35, []() -> IScriptObject* {return new struct DistanceFog(); }};
static ScriptObjectSpec FireFleaEnt = {0x36, []() -> IScriptObject* {return new struct FireFlea(); }};
static ScriptObjectSpec MetareeAlphaEnt = {0x37, []() -> IScriptObject* {return new struct MetareeAlpha(); }};
static ScriptObjectSpec DockAreaChangeEnt = {0x38, []() -> IScriptObject* {return new struct DockAreaChange(); }};
static ScriptObjectSpec ActorRotateEnt = {0x39, []() -> IScriptObject* {return new struct ActorRotate(); }};
static ScriptObjectSpec SpecialFunctionEnt = {0x3A, []() -> IScriptObject* {return new struct SpecialFunction(); }};
static ScriptObjectSpec SpankWeedEnt = {0x3B, []() -> IScriptObject* {return new struct SpankWeed(); }};
static ScriptObjectSpec ParasiteEnt = {0x3D, []() -> IScriptObject* {return new struct Parasite(); }};
static ScriptObjectSpec PlayerHintEnt = {0x3E, []() -> IScriptObject* {return new struct PlayerHint(); }};
static ScriptObjectSpec RipperEnt = {0x3F, []() -> IScriptObject* {return new struct Ripper(); }};
static ScriptObjectSpec PickupGeneratorEnt = {0x40, []() -> IScriptObject* {return new struct PickupGenerator(); }};
static ScriptObjectSpec AIKeyframeEnt = {0x41, []() -> IScriptObject* {return new struct AIKeyframe(); }};
static ScriptObjectSpec PointOfInterestEnt = {0x42, []() -> IScriptObject* {return new struct PointOfInterest(); }};
static ScriptObjectSpec DroneEnt = {0x43, []() -> IScriptObject* {return new struct Drone(); }};
static ScriptObjectSpec MetroidAlphaEnt = {0x44, []() -> IScriptObject* {return new struct MetroidAlpha(); }};
static ScriptObjectSpec DebrisExtendedEnt = {0x45, []() -> IScriptObject* {return new struct DebrisExtended(); }};
static ScriptObjectSpec SteamEnt = {0x46, []() -> IScriptObject* {return new struct Steam(); }};
static ScriptObjectSpec RippleEnt = {0x47, []() -> IScriptObject* {return new struct Ripple(); }};
static ScriptObjectSpec BallTriggerEnt = {0x48, []() -> IScriptObject* {return new struct BallTrigger(); }};
static ScriptObjectSpec TargetingPointEnt = {0x49, []() -> IScriptObject* {return new struct TargetingPoint(); }};
static ScriptObjectSpec ElectroMagneticPulseEnt = {0x4A, []() -> IScriptObject* {return new struct ElectroMagneticPulse(); }};
static ScriptObjectSpec IceSheegothEnt = {0x4B, []() -> IScriptObject* {return new struct IceSheegoth(); }};
static ScriptObjectSpec PlayerActorEnt = {0x4C, []() -> IScriptObject* {return new struct PlayerActor(); }};
static ScriptObjectSpec FlaahgraEnt = {0x4D, []() -> IScriptObject* {return new struct Flaahgra(); }};
static ScriptObjectSpec AreaAttributesEnt = {0x4E, []() -> IScriptObject* {return new struct AreaAttributes(); }};
static ScriptObjectSpec FishCloudEnt = {0x4F, []() -> IScriptObject* {return new struct FishCloud(); }};
static ScriptObjectSpec FishCloudModifierEnt = {0x50, []() -> IScriptObject* {return new struct FishCloudModifier(); }};
static ScriptObjectSpec VisorFlareEnt = {0x51, []() -> IScriptObject* {return new struct VisorFlare(); }};
static ScriptObjectSpec WorldTeleporterx52Ent = {0x52, []() -> IScriptObject* {return new struct WorldTeleporter(); }};
static ScriptObjectSpec VisorGooEnt = {0x53, []() -> IScriptObject* {return new struct VisorGoo(); }};
static ScriptObjectSpec JellyZapEnt = {0x54, []() -> IScriptObject* {return new struct JellyZap(); }};
static ScriptObjectSpec ControllerActionEnt = {0x55, []() -> IScriptObject* {return new struct ControllerAction(); }};
static ScriptObjectSpec SwitchEnt = {0x56, []() -> IScriptObject* {return new struct Switch(); }};
static ScriptObjectSpec PlayerStateChangeEnt = {0x57, []() -> IScriptObject* {return new struct PlayerStateChange(); }};
static ScriptObjectSpec ThardusEnt = {0x58, []() -> IScriptObject* {return new struct Thardus(); }};
static ScriptObjectSpec WallCrawlerSwarmEnt = {0x5A, []() -> IScriptObject* {return new struct WallCrawlerSwarm(); }};
static ScriptObjectSpec AIJumpPointEnt = {0x5B, []() -> IScriptObject* {return new struct AIJumpPoint(); }};
static ScriptObjectSpec FlaahgraTentacleEnt = {0x5C, []() -> IScriptObject* {return new struct FlaahgraTentacle(); }};
static ScriptObjectSpec RoomAcousticsEnt = {0x5D, []() -> IScriptObject* {return new struct RoomAcoustics(); }};
static ScriptObjectSpec ColorModulateEnt = {0x5E, []() -> IScriptObject* {return new struct ColorModulate(); }};
static ScriptObjectSpec ThardusRockProjectileEnt = {0x5F, []() -> IScriptObject* {return new struct ThardusRockProjectile(); }};
static ScriptObjectSpec MidiEnt = {0x60, []() -> IScriptObject* {return new struct Midi(); }};
static ScriptObjectSpec StreamedAudioEnt = {0x61, []() -> IScriptObject* {return new struct StreamedAudio(); }};
static ScriptObjectSpec WorldTeleporterx62Ent = {0x62, []() -> IScriptObject* {return new struct WorldTeleporter(); }}; // o.o, no this is not a trick
static ScriptObjectSpec RepulsorEnt = {0x63, []() -> IScriptObject* {return new struct Repulsor(); }};
static ScriptObjectSpec GunTurretEnt = {0x64, []() -> IScriptObject* {return new struct GunTurret(); }};
static ScriptObjectSpec FogVolumeEnt = {0x65, []() -> IScriptObject* {return new struct FogVolume(); }};
static ScriptObjectSpec BabygothEnt = {0x66, []() -> IScriptObject* {return new struct Babygoth(); }};
static ScriptObjectSpec EyeballEnt = {0x67, []() -> IScriptObject* {return new struct Eyeball(); }};
static ScriptObjectSpec RadialDamageEnt = {0x68, []() -> IScriptObject* {return new struct RadialDamage(); }};
static ScriptObjectSpec CameraPitchVolumeEnt = {0x69, []() -> IScriptObject* {return new struct CameraPitchVolume(); }};
static ScriptObjectSpec EnvFxDensityControllerEnt = {0x6A, []() -> IScriptObject* {return new struct EnvFxDensityController(); }};
static ScriptObjectSpec MagdoliteEnt = {0x6B, []() -> IScriptObject* {return new struct Magdolite(); }};
static ScriptObjectSpec TeamAIMgrEnt = {0x6C, []() -> IScriptObject* {return new struct TeamAIMgr(); }};
static ScriptObjectSpec SnakeWeedSwarmEnt = {0x6D, []() -> IScriptObject* {return new struct SnakeWeedSwarm(); }};
static ScriptObjectSpec ActorContraptionEnt = {0x6E, []() -> IScriptObject* {return new struct ActorContraption(); }};
static ScriptObjectSpec OculusEnt = {0x6F, []() -> IScriptObject* {return new struct Oculus(); }};
static ScriptObjectSpec GeemerEnt = {0x70, []() -> IScriptObject* {return new struct Geemer(); }};
static ScriptObjectSpec SpindleCameraEnt = {0x71, []() -> IScriptObject* {return new struct SpindleCamera(); }};
static ScriptObjectSpec AtomicAlphaEnt = {0x72, []() -> IScriptObject* {return new struct AtomicAlpha(); }};
static ScriptObjectSpec CameraHintTriggerEnt = {0x73, []() -> IScriptObject* {return new struct CameraHintTrigger(); }};
static ScriptObjectSpec RumbleEffectEnt = {0x74, []() -> IScriptObject* {return new struct RumbleEffect(); }};
static ScriptObjectSpec AmbientAIEnt = {0x75, []() -> IScriptObject* {return new struct AmbientAI(); }};
static ScriptObjectSpec AtomicBetaEnt = {0x77, []() -> IScriptObject* {return new struct AtomicBeta(); }};
static ScriptObjectSpec IceZoomerEnt = {0x78, []() -> IScriptObject* {return new struct IceZoomer(); }};
static ScriptObjectSpec PufferEnt = {0x79, []() -> IScriptObject* {return new struct Puffer(); }};
static ScriptObjectSpec TryclopsEnt = {0x7A, []() -> IScriptObject* {return new struct Tryclops(); }};
static ScriptObjectSpec RidleyEnt = {0x7B, []() -> IScriptObject* {return new struct Ridley(); }};
static ScriptObjectSpec SeedlingEnt = {0x7C, []() -> IScriptObject* {return new struct Seedling(); }};
static ScriptObjectSpec ThermalHeatFaderEnt = {0x7D, []() -> IScriptObject* {return new struct ThermalHeatFader(); }};
static ScriptObjectSpec BurrowerEnt = {0x7F, []() -> IScriptObject* {return new struct Burrower(); }};
static ScriptObjectSpec ScriptBeamEnt = {0x81, []() -> IScriptObject* {return new struct ScriptBeam(); }};
static ScriptObjectSpec WorldLightFaderEnt = {0x82, []() -> IScriptObject* {return new struct WorldLightFader(); }};
static ScriptObjectSpec MetroidPrimeStage2Ent = {0x83, []() -> IScriptObject* {return new struct MetroidPrimeStage2(); }};
static ScriptObjectSpec MetroidPrimeStage1Ent = {0x84, []() -> IScriptObject* {return new struct MetroidPrimeStage1(); }};
static ScriptObjectSpec MazeNodeEnt = {0x85, []() -> IScriptObject* {return new struct MazeNode(); }};
static ScriptObjectSpec OmegaPirateEnt = {0x86, []() -> IScriptObject* {return new struct OmegaPirate(); }};
static ScriptObjectSpec PhazonPoolEnt = {0x87, []() -> IScriptObject* {return new struct PhazonPool(); }};
static ScriptObjectSpec PhazonHealingNoduleEnt = {0x88, []() -> IScriptObject* {return new struct PhazonHealingNodule(); }};
static ScriptObjectSpec NewCameraShakerEnt = {0x89, []() -> IScriptObject* {return new struct NewCameraShaker(); }};
static ScriptObjectSpec ShadowProjectorEnt = {0x8A, []() -> IScriptObject* {return new struct ShadowProjector(); }};
static ScriptObjectSpec EnergyBallEnt = {0x8B, []() -> IScriptObject* {return new struct EnergyBall(); }};
}

std::vector<const struct ScriptObjectSpec*> SCRIPT_OBJECT_DB =
{
    &priv::AIJumpPointEnt,
    &priv::AIKeyframeEnt,
    &priv::ActorEnt,
    &priv::ActorContraptionEnt,
    &priv::ActorKeyframeEnt,
    &priv::ActorRotateEnt,
    &priv::AmbientAIEnt,
    &priv::AreaAttributesEnt,
    &priv::AtomicAlphaEnt,
    &priv::AtomicBetaEnt,
    &priv::BabygothEnt,
    &priv::BallTriggerEnt,
    &priv::BeetleEnt,
    &priv::BloodFlowerEnt,
    &priv::BurrowerEnt,
    &priv::CameraEnt,
    &priv::CameraBlurKeyframeEnt,
    &priv::CameraFilterKeyframeEnt,
    &priv::CameraHintEnt,
    &priv::CameraHintTriggerEnt,
    &priv::CameraPitchVolumeEnt,
    &priv::CameraShakerEnt,
    &priv::CameraWaypointEnt,
    &priv::ChozoGhostEnt,
    &priv::ColorModulateEnt,
    &priv::ControllerActionEnt,
    &priv::CounterEnt,
    &priv::CoverPointEnt,
    &priv::DamageableTriggerEnt,
    &priv::DebrisEnt,
    &priv::DebrisExtendedEnt,
    &priv::DebugCameraWaypointEnt,
    &priv::DistanceFogEnt,
    &priv::DockEnt,
    &priv::DockAreaChangeEnt,
    &priv::DoorAreaEnt,
    &priv::DroneEnt,
    &priv::EffectEnt,
    &priv::ElectroMagneticPulseEnt,
    &priv::ElitePirateEnt,
    &priv::EnergyBallEnt,
    &priv::EnvFxDensityControllerEnt,
    &priv::EyeballEnt,
    &priv::FireFleaEnt,
    &priv::FishCloudEnt,
    &priv::FishCloudModifierEnt,
    &priv::FlaahgraEnt,
    &priv::FlaahgraTentacleEnt,
    &priv::FlickerBatEnt,
    &priv::FlyingPirateEnt,
    &priv::FogVolumeEnt,
    &priv::GeemerEnt,
    &priv::GeneratorEnt,
    &priv::GrapplePointEnt,
    &priv::GunTurretEnt,
    &priv::HUDMemoEnt,
    &priv::IceSheegothEnt,
    &priv::IceZoomerEnt,
    &priv::JellyZapEnt,
    &priv::MagdoliteEnt,
    &priv::MazeNodeEnt,
    &priv::MemoryRelayEnt,
    &priv::MetareeAlphaEnt,
    &priv::MetroidAlphaEnt,
    &priv::MetroidBetaEnt,
    &priv::MetroidPrimeStage1Ent,
    &priv::MetroidPrimeStage2Ent,
    &priv::MidiEnt,
    &priv::NewCameraShakerEnt,
    &priv::NewIntroBossEnt,
    &priv::OculusEnt,
    &priv::OmegaPirateEnt,
    &priv::ParasiteEnt,
    &priv::PathCameraEnt,
    &priv::PhazonHealingNoduleEnt,
    &priv::PhazonPoolEnt,
    &priv::PickupEnt,
    &priv::PickupGeneratorEnt,
    &priv::PlatformEnt,
    &priv::PlayerActorEnt,
    &priv::PlayerHintEnt,
    &priv::PlayerStateChangeEnt,
    &priv::PointOfInterestEnt,
    &priv::PuddleSporeEnt,
    &priv::PuddleToadGammaEnt,
    &priv::PufferEnt,
    &priv::RadialDamageEnt,
    &priv::RandomRelayEnt,
    &priv::RelayEnt,
    &priv::RepulsorEnt,
    &priv::RidleyEnt,
    &priv::RipperEnt,
    &priv::RippleEnt,
    &priv::RoomAcousticsEnt,
    &priv::RumbleEffectEnt,
    &priv::ScriptBeamEnt,
    &priv::SeedlingEnt,
    &priv::ShadowProjectorEnt,
    &priv::SnakeWeedSwarmEnt,
    &priv::SoundEnt,
    &priv::SpacePirateEnt,
    &priv::SpankWeedEnt,
    &priv::SpawnPointEnt,
    &priv::SpecialFunctionEnt,
    &priv::SpiderBallAttractionSurfaceEnt,
    &priv::SpiderBallWaypointEnt,
    &priv::SpindleCameraEnt,
    &priv::SteamEnt,
    &priv::StreamedAudioEnt,
    &priv::SwitchEnt,
    &priv::TargetingPointEnt,
    &priv::TeamAIMgrEnt,
    &priv::ThardusEnt,
    &priv::ThardusRockProjectileEnt,
    &priv::ThermalHeatFaderEnt,
    &priv::TimerEnt,
    &priv::TriggerEnt,
    &priv::TryclopsEnt,
    &priv::VisorFlareEnt,
    &priv::VisorGooEnt,
    &priv::WallCrawlerSwarmEnt,
    &priv::WarwaspEnt,
    &priv::WaterEnt,
    &priv::WaypointEnt,
    &priv::WorldLightFaderEnt,
    &priv::WorldTeleporterx52Ent,
    &priv::WorldTeleporterx62Ent,
};

}
}
