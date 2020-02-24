#include "IScriptObject.hpp"

#include "Actor.hpp"
#include "ActorContraption.hpp"
#include "ActorKeyframe.hpp"
#include "ActorRotate.hpp"
#include "AIJumpPoint.hpp"
#include "AIKeyframe.hpp"
#include "AmbientAI.hpp"
#include "AreaAttributes.hpp"
#include "AtomicAlpha.hpp"
#include "AtomicBeta.hpp"
#include "Babygoth.hpp"
#include "BallTrigger.hpp"
#include "Beetle.hpp"
#include "BloodFlower.hpp"
#include "Burrower.hpp"
#include "Camera.hpp"
#include "CameraBlurKeyframe.hpp"
#include "CameraFilterKeyframe.hpp"
#include "CameraHint.hpp"
#include "CameraHintTrigger.hpp"
#include "CameraPitchVolume.hpp"
#include "CameraShaker.hpp"
#include "CameraWaypoint.hpp"
#include "ChozoGhost.hpp"
#include "ColorModulate.hpp"
#include "ControllerAction.hpp"
#include "Counter.hpp"
#include "CoverPoint.hpp"
#include "DamageableTrigger.hpp"
#include "Debris.hpp"
#include "DebrisExtended.hpp"
#include "DebugCameraWaypoint.hpp"
#include "DistanceFog.hpp"
#include "Dock.hpp"
#include "DockAreaChange.hpp"
#include "DoorArea.hpp"
#include "Drone.hpp"
#include "Effect.hpp"
#include "ElectroMagneticPulse.hpp"
#include "ElitePirate.hpp"
#include "EnergyBall.hpp"
#include "EnvFxDensityController.hpp"
#include "Eyeball.hpp"
#include "FireFlea.hpp"
#include "FishCloud.hpp"
#include "FishCloudModifier.hpp"
#include "Flaahgra.hpp"
#include "FlaahgraTentacle.hpp"
#include "FlickerBat.hpp"
#include "FlyingPirate.hpp"
#include "FogVolume.hpp"
#include "Geemer.hpp"
#include "Generator.hpp"
#include "GrapplePoint.hpp"
#include "GunTurret.hpp"
#include "HUDMemo.hpp"
#include "IceSheegoth.hpp"
#include "IceZoomer.hpp"
#include "JellyZap.hpp"
#include "Magdolite.hpp"
#include "MazeNode.hpp"
#include "MemoryRelay.hpp"
#include "MetareeAlpha.hpp"
#include "MetroidAlpha.hpp"
#include "MetroidBeta.hpp"
#include "MetroidPrimeStage1.hpp"
#include "MetroidPrimeStage2.hpp"
#include "Midi.hpp"
#include "NewCameraShaker.hpp"
#include "NewIntroBoss.hpp"
#include "Oculus.hpp"
#include "OmegaPirate.hpp"
#include "Parasite.hpp"
#include "PathCamera.hpp"
#include "PhazonHealingNodule.hpp"
#include "PhazonPool.hpp"
#include "Pickup.hpp"
#include "PickupGenerator.hpp"
#include "Platform.hpp"
#include "PlayerActor.hpp"
#include "PlayerHint.hpp"
#include "PlayerStateChange.hpp"
#include "PointOfInterest.hpp"
#include "PuddleSpore.hpp"
#include "PuddleToadGamma.hpp"
#include "Puffer.hpp"
#include "RadialDamage.hpp"
#include "RandomRelay.hpp"
#include "Relay.hpp"
#include "Repulsor.hpp"
#include "Ridley.hpp"
#include "Ripper.hpp"
#include "Ripple.hpp"
#include "RoomAcoustics.hpp"
#include "RumbleEffect.hpp"
#include "ScriptBeam.hpp"
#include "ScriptTypes.hpp"
#include "Seedling.hpp"
#include "ShadowProjector.hpp"
#include "SnakeWeedSwarm.hpp"
#include "Sound.hpp"
#include "SpacePirate.hpp"
#include "SpankWeed.hpp"
#include "SpawnPoint.hpp"
#include "SpecialFunction.hpp"
#include "SpiderBallAttractionSurface.hpp"
#include "SpiderBallWaypoint.hpp"
#include "SpindleCamera.hpp"
#include "Steam.hpp"
#include "StreamedAudio.hpp"
#include "Switch.hpp"
#include "TargetingPoint.hpp"
#include "TeamAIMgr.hpp"
#include "Thardus.hpp"
#include "ThardusRockProjectile.hpp"
#include "ThermalHeatFader.hpp"
#include "Timer.hpp"
#include "Trigger.hpp"
#include "Tryclops.hpp"
#include "VisorFlare.hpp"
#include "VisorGoo.hpp"
#include "WallCrawlerSwarm.hpp"
#include "Warwasp.hpp"
#include "Water.hpp"
#include "Waypoint.hpp"
#include "WorldLightFader.hpp"
#include "WorldTeleporter.hpp"

namespace DataSpec::DNAMP1 {
namespace priv {
namespace {
constexpr ScriptObjectSpec ActorEnt = {0x00, []() -> IScriptObject* { return new struct Actor(); }};
constexpr ScriptObjectSpec WaypointEnt = {0x02, []() -> IScriptObject* { return new struct Waypoint(); }};
constexpr ScriptObjectSpec DoorAreaEnt = {0x03, []() -> IScriptObject* { return new struct DoorArea(); }};
constexpr ScriptObjectSpec TriggerEnt = {0x04, []() -> IScriptObject* { return new struct Trigger(); }};
constexpr ScriptObjectSpec TimerEnt = {0x05, []() -> IScriptObject* { return new struct Timer(); }};
constexpr ScriptObjectSpec CounterEnt = {0x06, []() -> IScriptObject* { return new struct Counter(); }};
constexpr ScriptObjectSpec EffectEnt = {0x07, []() -> IScriptObject* { return new struct Effect(); }};
constexpr ScriptObjectSpec PlatformEnt = {0x08, []() -> IScriptObject* { return new struct Platform(); }};
constexpr ScriptObjectSpec SoundEnt = {0x09, []() -> IScriptObject* { return new struct Sound(); }};
constexpr ScriptObjectSpec GeneratorEnt = {0x0A, []() -> IScriptObject* { return new struct Generator(); }};
constexpr ScriptObjectSpec DockEnt = {0x0B, []() -> IScriptObject* { return new struct Dock(); }};
constexpr ScriptObjectSpec CameraEnt = {0x0C, []() -> IScriptObject* { return new struct Camera(); }};
constexpr ScriptObjectSpec CameraWaypointEnt = {0x0D, []() -> IScriptObject* { return new struct CameraWaypoint(); }};
constexpr ScriptObjectSpec NewIntroBossEnt = {0x0E, []() -> IScriptObject* { return new struct NewIntroBoss(); }};
constexpr ScriptObjectSpec SpawnPointEnt = {0x0F, []() -> IScriptObject* { return new struct SpawnPoint(); }};
constexpr ScriptObjectSpec CameraHintEnt = {0x10, []() -> IScriptObject* { return new struct CameraHint(); }};
constexpr ScriptObjectSpec PickupEnt = {0x11, []() -> IScriptObject* { return new struct Pickup(); }};
constexpr ScriptObjectSpec MemoryRelayEnt = {0x13, []() -> IScriptObject* { return new struct MemoryRelay(); }};
constexpr ScriptObjectSpec RandomRelayEnt = {0x14, []() -> IScriptObject* { return new struct RandomRelay(); }};
constexpr ScriptObjectSpec RelayEnt = {0x15, []() -> IScriptObject* { return new struct Relay(); }};
constexpr ScriptObjectSpec BeetleEnt = {0x16, []() -> IScriptObject* { return new struct Beetle(); }};
constexpr ScriptObjectSpec HUDMemoEnt = {0x17, []() -> IScriptObject* { return new struct HUDMemo(); }};
constexpr ScriptObjectSpec CameraFilterKeyframeEnt = {
    0x18, []() -> IScriptObject* { return new struct CameraFilterKeyframe(); }};
constexpr ScriptObjectSpec CameraBlurKeyframeEnt = {0x19,
                                                    []() -> IScriptObject* { return new struct CameraBlurKeyframe(); }};
constexpr ScriptObjectSpec DamageableTriggerEnt = {0x1A,
                                                   []() -> IScriptObject* { return new struct DamageableTrigger(); }};
constexpr ScriptObjectSpec DebrisEnt = {0x1B, []() -> IScriptObject* { return new struct Debris(); }};
constexpr ScriptObjectSpec CameraShakerEnt = {0x1C, []() -> IScriptObject* { return new struct CameraShaker(); }};
constexpr ScriptObjectSpec ActorKeyframeEnt = {0x1D, []() -> IScriptObject* { return new struct ActorKeyframe(); }};
constexpr ScriptObjectSpec WaterEnt = {0x20, []() -> IScriptObject* { return new struct Water(); }};
constexpr ScriptObjectSpec WarwaspEnt = {0x21, []() -> IScriptObject* { return new struct Warwasp(); }};
constexpr ScriptObjectSpec SpacePirateEnt = {0x24, []() -> IScriptObject* { return new struct SpacePirate(); }};
constexpr ScriptObjectSpec FlyingPirateEnt = {0x25, []() -> IScriptObject* { return new struct FlyingPirate(); }};
constexpr ScriptObjectSpec ElitePirateEnt = {0x26, []() -> IScriptObject* { return new struct ElitePirate(); }};
constexpr ScriptObjectSpec MetroidBetaEnt = {0x27, []() -> IScriptObject* { return new struct MetroidBeta(); }};
constexpr ScriptObjectSpec ChozoGhostEnt = {0x28, []() -> IScriptObject* { return new struct ChozoGhost(); }};
constexpr ScriptObjectSpec CoverPointEnt = {0x2A, []() -> IScriptObject* { return new struct CoverPoint(); }};
constexpr ScriptObjectSpec SpiderBallWaypointEnt = {0x2C,
                                                    []() -> IScriptObject* { return new struct SpiderBallWaypoint(); }};
constexpr ScriptObjectSpec BloodFlowerEnt = {0x2D, []() -> IScriptObject* { return new struct BloodFlower(); }};
constexpr ScriptObjectSpec FlickerBatEnt = {0x2E, []() -> IScriptObject* { return new struct FlickerBat(); }};
constexpr ScriptObjectSpec PathCameraEnt = {0x2F, []() -> IScriptObject* { return new struct PathCamera(); }};
constexpr ScriptObjectSpec GrapplePointEnt = {0x30, []() -> IScriptObject* { return new struct GrapplePoint(); }};
constexpr ScriptObjectSpec PuddleSporeEnt = {0x31, []() -> IScriptObject* { return new struct PuddleSpore(); }};
constexpr ScriptObjectSpec DebugCameraWaypointEnt = {
    0x32, []() -> IScriptObject* { return new struct DebugCameraWaypoint(); }};
constexpr ScriptObjectSpec SpiderBallAttractionSurfaceEnt = {
    0x33, []() -> IScriptObject* { return new struct SpiderBallAttractionSurface(); }};
constexpr ScriptObjectSpec PuddleToadGammaEnt = {0x34, []() -> IScriptObject* { return new struct PuddleToadGamma(); }};
constexpr ScriptObjectSpec DistanceFogEnt = {0x35, []() -> IScriptObject* { return new struct DistanceFog(); }};
constexpr ScriptObjectSpec FireFleaEnt = {0x36, []() -> IScriptObject* { return new struct FireFlea(); }};
constexpr ScriptObjectSpec MetareeAlphaEnt = {0x37, []() -> IScriptObject* { return new struct MetareeAlpha(); }};
constexpr ScriptObjectSpec DockAreaChangeEnt = {0x38, []() -> IScriptObject* { return new struct DockAreaChange(); }};
constexpr ScriptObjectSpec ActorRotateEnt = {0x39, []() -> IScriptObject* { return new struct ActorRotate(); }};
constexpr ScriptObjectSpec SpecialFunctionEnt = {0x3A, []() -> IScriptObject* { return new struct SpecialFunction(); }};
constexpr ScriptObjectSpec SpankWeedEnt = {0x3B, []() -> IScriptObject* { return new struct SpankWeed(); }};
constexpr ScriptObjectSpec ParasiteEnt = {0x3D, []() -> IScriptObject* { return new struct Parasite(); }};
constexpr ScriptObjectSpec PlayerHintEnt = {0x3E, []() -> IScriptObject* { return new struct PlayerHint(); }};
constexpr ScriptObjectSpec RipperEnt = {0x3F, []() -> IScriptObject* { return new struct Ripper(); }};
constexpr ScriptObjectSpec PickupGeneratorEnt = {0x40, []() -> IScriptObject* { return new struct PickupGenerator(); }};
constexpr ScriptObjectSpec AIKeyframeEnt = {0x41, []() -> IScriptObject* { return new struct AIKeyframe(); }};
constexpr ScriptObjectSpec PointOfInterestEnt = {0x42, []() -> IScriptObject* { return new struct PointOfInterest(); }};
constexpr ScriptObjectSpec DroneEnt = {0x43, []() -> IScriptObject* { return new struct Drone(); }};
constexpr ScriptObjectSpec MetroidAlphaEnt = {0x44, []() -> IScriptObject* { return new struct MetroidAlpha(); }};
constexpr ScriptObjectSpec DebrisExtendedEnt = {0x45, []() -> IScriptObject* { return new struct DebrisExtended(); }};
constexpr ScriptObjectSpec SteamEnt = {0x46, []() -> IScriptObject* { return new struct Steam(); }};
constexpr ScriptObjectSpec RippleEnt = {0x47, []() -> IScriptObject* { return new struct Ripple(); }};
constexpr ScriptObjectSpec BallTriggerEnt = {0x48, []() -> IScriptObject* { return new struct BallTrigger(); }};
constexpr ScriptObjectSpec TargetingPointEnt = {0x49, []() -> IScriptObject* { return new struct TargetingPoint(); }};
constexpr ScriptObjectSpec ElectroMagneticPulseEnt = {
    0x4A, []() -> IScriptObject* { return new struct ElectroMagneticPulse(); }};
constexpr ScriptObjectSpec IceSheegothEnt = {0x4B, []() -> IScriptObject* { return new struct IceSheegoth(); }};
constexpr ScriptObjectSpec PlayerActorEnt = {0x4C, []() -> IScriptObject* { return new struct PlayerActor(); }};
constexpr ScriptObjectSpec FlaahgraEnt = {0x4D, []() -> IScriptObject* { return new struct Flaahgra(); }};
constexpr ScriptObjectSpec AreaAttributesEnt = {0x4E, []() -> IScriptObject* { return new struct AreaAttributes(); }};
constexpr ScriptObjectSpec FishCloudEnt = {0x4F, []() -> IScriptObject* { return new struct FishCloud(); }};
constexpr ScriptObjectSpec FishCloudModifierEnt = {0x50,
                                                   []() -> IScriptObject* { return new struct FishCloudModifier(); }};
constexpr ScriptObjectSpec VisorFlareEnt = {0x51, []() -> IScriptObject* { return new struct VisorFlare(); }};
constexpr ScriptObjectSpec WorldTeleporterx52Ent = {0x52,
                                                    []() -> IScriptObject* { return new struct WorldTeleporter(); }};
constexpr ScriptObjectSpec VisorGooEnt = {0x53, []() -> IScriptObject* { return new struct VisorGoo(); }};
constexpr ScriptObjectSpec JellyZapEnt = {0x54, []() -> IScriptObject* { return new struct JellyZap(); }};
constexpr ScriptObjectSpec ControllerActionEnt = {0x55,
                                                  []() -> IScriptObject* { return new struct ControllerAction(); }};
constexpr ScriptObjectSpec SwitchEnt = {0x56, []() -> IScriptObject* { return new struct Switch(); }};
constexpr ScriptObjectSpec PlayerStateChangeEnt = {0x57,
                                                   []() -> IScriptObject* { return new struct PlayerStateChange(); }};
constexpr ScriptObjectSpec ThardusEnt = {0x58, []() -> IScriptObject* { return new struct Thardus(); }};
constexpr ScriptObjectSpec WallCrawlerSwarmEnt = {0x5A,
                                                  []() -> IScriptObject* { return new struct WallCrawlerSwarm(); }};
constexpr ScriptObjectSpec AIJumpPointEnt = {0x5B, []() -> IScriptObject* { return new struct AIJumpPoint(); }};
constexpr ScriptObjectSpec FlaahgraTentacleEnt = {0x5C,
                                                  []() -> IScriptObject* { return new struct FlaahgraTentacle(); }};
constexpr ScriptObjectSpec RoomAcousticsEnt = {0x5D, []() -> IScriptObject* { return new struct RoomAcoustics(); }};
constexpr ScriptObjectSpec ColorModulateEnt = {0x5E, []() -> IScriptObject* { return new struct ColorModulate(); }};
constexpr ScriptObjectSpec ThardusRockProjectileEnt = {
    0x5F, []() -> IScriptObject* { return new struct ThardusRockProjectile(); }};
constexpr ScriptObjectSpec MidiEnt = {0x60, []() -> IScriptObject* { return new struct Midi(); }};
constexpr ScriptObjectSpec StreamedAudioEnt = {0x61, []() -> IScriptObject* { return new struct StreamedAudio(); }};
constexpr ScriptObjectSpec WorldTeleporterx62Ent = {
    0x62, []() -> IScriptObject* { return new struct WorldTeleporter(); }}; // o.o, no this is not a trick
constexpr ScriptObjectSpec RepulsorEnt = {0x63, []() -> IScriptObject* { return new struct Repulsor(); }};
constexpr ScriptObjectSpec GunTurretEnt = {0x64, []() -> IScriptObject* { return new struct GunTurret(); }};
constexpr ScriptObjectSpec FogVolumeEnt = {0x65, []() -> IScriptObject* { return new struct FogVolume(); }};
constexpr ScriptObjectSpec BabygothEnt = {0x66, []() -> IScriptObject* { return new struct Babygoth(); }};
constexpr ScriptObjectSpec EyeballEnt = {0x67, []() -> IScriptObject* { return new struct Eyeball(); }};
constexpr ScriptObjectSpec RadialDamageEnt = {0x68, []() -> IScriptObject* { return new struct RadialDamage(); }};
constexpr ScriptObjectSpec CameraPitchVolumeEnt = {0x69,
                                                   []() -> IScriptObject* { return new struct CameraPitchVolume(); }};
constexpr ScriptObjectSpec EnvFxDensityControllerEnt = {
    0x6A, []() -> IScriptObject* { return new struct EnvFxDensityController(); }};
constexpr ScriptObjectSpec MagdoliteEnt = {0x6B, []() -> IScriptObject* { return new struct Magdolite(); }};
constexpr ScriptObjectSpec TeamAIMgrEnt = {0x6C, []() -> IScriptObject* { return new struct TeamAIMgr(); }};
constexpr ScriptObjectSpec SnakeWeedSwarmEnt = {0x6D, []() -> IScriptObject* { return new struct SnakeWeedSwarm(); }};
constexpr ScriptObjectSpec ActorContraptionEnt = {0x6E,
                                                  []() -> IScriptObject* { return new struct ActorContraption(); }};
constexpr ScriptObjectSpec OculusEnt = {0x6F, []() -> IScriptObject* { return new struct Oculus(); }};
constexpr ScriptObjectSpec GeemerEnt = {0x70, []() -> IScriptObject* { return new struct Geemer(); }};
constexpr ScriptObjectSpec SpindleCameraEnt = {0x71, []() -> IScriptObject* { return new struct SpindleCamera(); }};
constexpr ScriptObjectSpec AtomicAlphaEnt = {0x72, []() -> IScriptObject* { return new struct AtomicAlpha(); }};
constexpr ScriptObjectSpec CameraHintTriggerEnt = {0x73,
                                                   []() -> IScriptObject* { return new struct CameraHintTrigger(); }};
constexpr ScriptObjectSpec RumbleEffectEnt = {0x74, []() -> IScriptObject* { return new struct RumbleEffect(); }};
constexpr ScriptObjectSpec AmbientAIEnt = {0x75, []() -> IScriptObject* { return new struct AmbientAI(); }};
constexpr ScriptObjectSpec AtomicBetaEnt = {0x77, []() -> IScriptObject* { return new struct AtomicBeta(); }};
constexpr ScriptObjectSpec IceZoomerEnt = {0x78, []() -> IScriptObject* { return new struct IceZoomer(); }};
constexpr ScriptObjectSpec PufferEnt = {0x79, []() -> IScriptObject* { return new struct Puffer(); }};
constexpr ScriptObjectSpec TryclopsEnt = {0x7A, []() -> IScriptObject* { return new struct Tryclops(); }};
constexpr ScriptObjectSpec RidleyEnt = {0x7B, []() -> IScriptObject* { return new struct Ridley(); }};
constexpr ScriptObjectSpec SeedlingEnt = {0x7C, []() -> IScriptObject* { return new struct Seedling(); }};
constexpr ScriptObjectSpec ThermalHeatFaderEnt = {0x7D,
                                                  []() -> IScriptObject* { return new struct ThermalHeatFader(); }};
constexpr ScriptObjectSpec BurrowerEnt = {0x7F, []() -> IScriptObject* { return new struct Burrower(); }};
constexpr ScriptObjectSpec ScriptBeamEnt = {0x81, []() -> IScriptObject* { return new struct ScriptBeam(); }};
constexpr ScriptObjectSpec WorldLightFaderEnt = {0x82, []() -> IScriptObject* { return new struct WorldLightFader(); }};
constexpr ScriptObjectSpec MetroidPrimeStage2Ent = {0x83,
                                                    []() -> IScriptObject* { return new struct MetroidPrimeStage2(); }};
constexpr ScriptObjectSpec MetroidPrimeStage1Ent = {0x84,
                                                    []() -> IScriptObject* { return new struct MetroidPrimeStage1(); }};
constexpr ScriptObjectSpec MazeNodeEnt = {0x85, []() -> IScriptObject* { return new struct MazeNode(); }};
constexpr ScriptObjectSpec OmegaPirateEnt = {0x86, []() -> IScriptObject* { return new struct OmegaPirate(); }};
constexpr ScriptObjectSpec PhazonPoolEnt = {0x87, []() -> IScriptObject* { return new struct PhazonPool(); }};
constexpr ScriptObjectSpec PhazonHealingNoduleEnt = {
    0x88, []() -> IScriptObject* { return new struct PhazonHealingNodule(); }};
constexpr ScriptObjectSpec NewCameraShakerEnt = {0x89, []() -> IScriptObject* { return new struct NewCameraShaker(); }};
constexpr ScriptObjectSpec ShadowProjectorEnt = {0x8A, []() -> IScriptObject* { return new struct ShadowProjector(); }};
constexpr ScriptObjectSpec EnergyBallEnt = {0x8B, []() -> IScriptObject* { return new struct EnergyBall(); }};
} // Anonymous namespace
} // namespace priv

const std::vector<const struct ScriptObjectSpec*> SCRIPT_OBJECT_DB = {
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

zeus::CTransform ConvertEditorEulerToTransform4f(const zeus::CVector3f& scale, const zeus::CVector3f& orientation,
                                                 const zeus::CVector3f& position) {
  zeus::simd_floats f(orientation.mSimd);
  return zeus::CTransform::RotateZ(zeus::degToRad(f[2])) * zeus::CTransform::RotateY(zeus::degToRad(f[1])) *
             zeus::CTransform::RotateX(zeus::degToRad(f[0])) * zeus::CTransform::Scale(scale) +
         position;
}

} // namespace DataSpec::DNAMP1
