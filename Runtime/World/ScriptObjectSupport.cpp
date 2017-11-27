#include "ScriptObjectSupport.hpp"

using namespace std::literals;

namespace urde
{

#ifndef NDEBUG
std::string_view ScriptObjectTypeToStr(EScriptObjectType type)
{
    switch (type)
    { 
    case EScriptObjectType::Actor: return "Actor"sv;
    case EScriptObjectType::Waypoint: return "Waypoint"sv;
    case EScriptObjectType::Door: return "Door"sv;
    case EScriptObjectType::Trigger: return "Trigger"sv;
    case EScriptObjectType::Timer: return "Timer"sv;
    case EScriptObjectType::Counter: return "Counter"sv;
    case EScriptObjectType::Effect: return "Effect"sv;
    case EScriptObjectType::Platform: return "Platform"sv;
    case EScriptObjectType::Sound: return "Sound"sv;
    case EScriptObjectType::Generator: return "Generator"sv;
    case EScriptObjectType::Dock: return "Dock"sv;
    case EScriptObjectType::Camera: return "Camera"sv;
    case EScriptObjectType::CameraWaypoint: return "CameraWaypoint"sv;
    case EScriptObjectType::NewIntroBoss: return "NewIntroBoss"sv;
    case EScriptObjectType::SpawnPoint: return "SpawnPoint"sv;
    case EScriptObjectType::CameraHint: return "CameraHint"sv;
    case EScriptObjectType::Pickup: return "Pickup"sv;
    case EScriptObjectType::MemoryRelay: return "MemoryRelay"sv;
    case EScriptObjectType::RandomRelay: return "RandomRelay"sv;
    case EScriptObjectType::Relay: return "Relay"sv;
    case EScriptObjectType::Beetle: return "Beetle"sv;
    case EScriptObjectType::HUDMemo: return "HUDMemo"sv;
    case EScriptObjectType::CameraFilterKeyframe: return "CameraFilterKeyframe"sv;
    case EScriptObjectType::CameraBlurKeyframe: return "CameraBlurKeyframe"sv;
    case EScriptObjectType::DamageableTrigger: return "DamageableTrigger"sv;
    case EScriptObjectType::Debris: return "Debris"sv;
    case EScriptObjectType::CameraShaker: return "CameraShaker"sv;
    case EScriptObjectType::ActorKeyframe: return "ActorKeyframe"sv;
    case EScriptObjectType::Water: return "Water"sv;
    case EScriptObjectType::Warwasp: return "Warwasp"sv;
    case EScriptObjectType::SpacePirate: return "SpacePirate"sv;
    case EScriptObjectType::FlyingPirate: return "FlyingPirate"sv;
    case EScriptObjectType::ElitePirate: return "ElitePirate"sv;
    case EScriptObjectType::MetroidBeta: return "MetroidBeta"sv;
    case EScriptObjectType::ChozoGhost: return "ChozoGhost"sv;
    case EScriptObjectType::CoverPoint: return "CoverPoint"sv;
    case EScriptObjectType::SpiderBallWaypoint: return "SpiderBallWaypoint"sv;
    case EScriptObjectType::BloodFlower: return "BloodFlower"sv;
    case EScriptObjectType::FlickerBat: return "FlickerBat"sv;
    case EScriptObjectType::PathCamera: return "PathCamera"sv;
    case EScriptObjectType::GrapplePoint: return "GrapplePoint"sv;
    case EScriptObjectType::PuddleSpore: return "PuddleSpore"sv;
    case EScriptObjectType::DebugCameraWaypoint: return "DebugCameraWaypoint"sv;
    case EScriptObjectType::SpiderBallAttractionSurface: return "SpiderBallAttractionSurface"sv;
    case EScriptObjectType::PuddleToadGamma: return "PuddleToadGamma"sv;
    case EScriptObjectType::DistanceFog: return "DistanceFog"sv;
    case EScriptObjectType::FireFlea: return "FireFlea"sv;
    case EScriptObjectType::MetareeAlpha: return "MetareeAlpha"sv;
    case EScriptObjectType::DockAreaChange: return "DockAreaChange"sv;
    case EScriptObjectType::ActorRotate: return "ActorRotate"sv;
    case EScriptObjectType::SpecialFunction: return "SpecialFunction"sv;
    case EScriptObjectType::SpankWeed: return "SpankWeed"sv;
    case EScriptObjectType::Parasite: return "Parasite"sv;
    case EScriptObjectType::PlayerHint: return "PlayerHint"sv;
    case EScriptObjectType::Ripper: return "Ripper"sv;
    case EScriptObjectType::PickupGenerator: return "PickupGenerator"sv;
    case EScriptObjectType::AIKeyframe: return "AIKeyframe"sv;
    case EScriptObjectType::PointOfInterest: return "PointOfInterest"sv;
    case EScriptObjectType::Drone: return "Drone"sv;
    case EScriptObjectType::MetroidAlpha: return "MetroidAlpha"sv;
    case EScriptObjectType::DebrisExtended: return "DebrisExtended"sv;
    case EScriptObjectType::Steam: return "Steam"sv;
    case EScriptObjectType::Ripple: return "Ripple"sv;
    case EScriptObjectType::BallTrigger: return "BallTrigger"sv;
    case EScriptObjectType::TargetingPoint: return "TargetingPoint"sv;
    case EScriptObjectType::EMPulse: return "EMPulse"sv;
    case EScriptObjectType::IceSheegoth: return "IceSheegoth"sv;
    case EScriptObjectType::PlayerActor: return "PlayerActor"sv;
    case EScriptObjectType::Flaahgra: return "Flaahgra"sv;
    case EScriptObjectType::AreaAttributes: return "AreaAttributes"sv;
    case EScriptObjectType::FishCloud: return "FishCloud"sv;
    case EScriptObjectType::FishCloudModifier: return "FishCloudModifier"sv;
    case EScriptObjectType::VisorFlare: return "VisorFlare"sv;
    case EScriptObjectType::WorldTeleporter: return "WorldTeleporter"sv;
    case EScriptObjectType::VisorGoo: return "VisorGoo"sv;
    case EScriptObjectType::JellyZap: return "JellyZap"sv;
    case EScriptObjectType::ControllerAction: return "ControllerAction"sv;
    case EScriptObjectType::Switch: return "Switch"sv;
    case EScriptObjectType::PlayerStateChange: return "PlayerStateChange"sv;
    case EScriptObjectType::Thardus: return "Thardus"sv;
    case EScriptObjectType::WallCrawlerSwarm: return "WallCrawlerSwarm"sv;
    case EScriptObjectType::AIJumpPoint: return "AIJumpPoint"sv;
    case EScriptObjectType::FlaahgraTentacle: return "FlaahgraTentacle"sv;
    case EScriptObjectType::RoomAcoustics: return "RoomAcoustics"sv;
    case EScriptObjectType::ColorModulate: return "ColorModulate"sv;
    case EScriptObjectType::ThardusRockProjectile: return "ThardusRockProjectile"sv;
    case EScriptObjectType::Midi: return "Midi"sv;
    case EScriptObjectType::StreamedAudio: return "StreamedAudio"sv;
    case EScriptObjectType::WorldTeleporterToo: return "WorldTeleporterToo"sv;
    case EScriptObjectType::Repulsor: return "Repulsor"sv;
    case EScriptObjectType::GunTurret: return "GunTurret"sv;
    case EScriptObjectType::FogVolume: return "FogVolume"sv;
    case EScriptObjectType::Babygoth: return "Babygoth"sv;
    case EScriptObjectType::Eyeball: return "Eyeball"sv;
    case EScriptObjectType::RadialDamage: return "RadialDamage"sv;
    case EScriptObjectType::CameraPitchVolume: return "CameraPitchVolume"sv;
    case EScriptObjectType::EnvFxDensityController: return "EnvFxDensityController"sv;
    case EScriptObjectType::Magdolite: return "Magdolite"sv;
    case EScriptObjectType::TeamAIMgr: return "TeamAIMgr"sv;
    case EScriptObjectType::SnakeWeedSwarm: return "SnakeWeedSwarm"sv;
    case EScriptObjectType::ActorContraption: return "ActorContraption"sv;
    case EScriptObjectType::Oculus: return "Oculus"sv;
    case EScriptObjectType::Geemer: return "Geemer"sv;
    case EScriptObjectType::SpindleCamera: return "SpindleCamera"sv;
    case EScriptObjectType::AtomicAlpha: return "AtomicAlpha"sv;
    case EScriptObjectType::CameraHintTrigger: return "CameraHintTrigger"sv;
    case EScriptObjectType::RumbleEffect: return "RumbleEffect"sv;
    case EScriptObjectType::AmbientAI: return "AmbientAI"sv;
    case EScriptObjectType::AtomicBeta: return "AtomicBeta"sv;
    case EScriptObjectType::IceZoomer: return "IceZoomer"sv;
    case EScriptObjectType::Puffer: return "Puffer"sv;
    case EScriptObjectType::Tryclops: return "Tryclops"sv;
    case EScriptObjectType::Ridley: return "Ridley"sv;
    case EScriptObjectType::Seedling: return "Seedling"sv;
    case EScriptObjectType::ThermalHeatFader: return "ThermalHeatFader"sv;
    case EScriptObjectType::Burrower: return "Burrower"sv;
    case EScriptObjectType::ScriptBeam: return "ScriptBeam"sv;
    case EScriptObjectType::WorldLightFader: return "WorldLightFader"sv;
    case EScriptObjectType::MetroidPrimeStage2: return "MetroidPrimeStage2"sv;
    case EScriptObjectType::MetroidPrimeStage1: return "MetroidPrimeStage1"sv;
    case EScriptObjectType::MazeNode: return "MazeNode"sv;
    case EScriptObjectType::OmegaPirate: return "OmegaPirate"sv;
    case EScriptObjectType::PhazonPool: return "PhazonPool"sv;
    case EScriptObjectType::PhazonHealingNodule: return "PhazonHealingNodule"sv;
    case EScriptObjectType::NewCameraShaker: return "NewCameraShaker"sv;
    case EScriptObjectType::ShadowProjector: return "ShadowProjector"sv;
    case EScriptObjectType::EnergyBall: return "EnergyBall"sv;
    default: return "..."sv;
    }
}

std::string_view ScriptObjectStateToStr(EScriptObjectState state)
{
    switch (state)
    {
    case EScriptObjectState::Active: return "Active"sv;
    case EScriptObjectState::Arrived: return "Arrived"sv;
    case EScriptObjectState::Closed: return "Closed"sv;
    case EScriptObjectState::Entered: return "Entered"sv;
    case EScriptObjectState::Exited: return "Exited"sv;
    case EScriptObjectState::Inactive: return "Inactive"sv;
    case EScriptObjectState::Inside: return "Inside"sv;
    case EScriptObjectState::MaxReached: return "MaxReached"sv;
    case EScriptObjectState::Open: return "Open"sv;
    case EScriptObjectState::Zero: return "Zero"sv;
    case EScriptObjectState::Attack: return "Attack"sv;
    case EScriptObjectState::UNKS1: return "UNKS1"sv;
    case EScriptObjectState::Retreat: return "Retreat"sv;
    case EScriptObjectState::Patrol: return "Patrol"sv;
    case EScriptObjectState::Dead: return "Dead"sv;
    case EScriptObjectState::CameraPath: return "CameraPath"sv;
    case EScriptObjectState::CameraTarget: return "CameraTarget"sv;
    case EScriptObjectState::UNKS2: return "UNKS2"sv;
    case EScriptObjectState::Play: return "Play"sv;
    case EScriptObjectState::UNKS3: return "UNKS3"sv;
    case EScriptObjectState::DeathRattle: return "DeathRattle"sv;
    case EScriptObjectState::UNKS4: return "UNKS4"sv;
    case EScriptObjectState::Damage: return "Damage"sv;
    case EScriptObjectState::InvulnDamage: return "InvulnDamage"sv;
    case EScriptObjectState::UNKS5: return "UNKS5"sv;
    case EScriptObjectState::Modify: return "Modify"sv;
    case EScriptObjectState::ScanStart: return "ScanStart"sv;
    case EScriptObjectState::ScanProcessing: return "ScanProcessing"sv;
    case EScriptObjectState::ScanDone: return "ScanDone"sv;
    case EScriptObjectState::UnFrozen: return "UnFrozen"sv;
    case EScriptObjectState::Default: return "Default"sv;
    case EScriptObjectState::ReflectedDamage: return "ReflectedDamage"sv;
    case EScriptObjectState::InheritBounds: return "InheritBounds"sv;
    default: return "..."sv;
    }
}

std::string_view ScriptObjectMessageToStr(EScriptObjectMessage message)
{
    switch (message)
    {
    case EScriptObjectMessage::UNKM0: return "UNKM0"sv;
    case EScriptObjectMessage::Activate: return "Activate"sv;
    case EScriptObjectMessage::Arrived: return "Arrived"sv;
    case EScriptObjectMessage::Close: return "Close"sv;
    case EScriptObjectMessage::Deactivate: return "Deactivate"sv;
    case EScriptObjectMessage::Decrement: return "Decrement"sv;
    case EScriptObjectMessage::Follow: return "Follow"sv;
    case EScriptObjectMessage::Increment: return "Increment"sv;
    case EScriptObjectMessage::Next: return "Next"sv;
    case EScriptObjectMessage::Open: return "Open"sv;
    case EScriptObjectMessage::Reset: return "Reset"sv;
    case EScriptObjectMessage::ResetAndStart: return "ResetAndStart"sv;
    case EScriptObjectMessage::SetToMax: return "SetToMax"sv;
    case EScriptObjectMessage::SetToZero: return "SetToZero"sv;
    case EScriptObjectMessage::Start: return "Start"sv;
    case EScriptObjectMessage::Stop: return "Stop"sv;
    case EScriptObjectMessage::StopAndReset: return "StopAndReset"sv;
    case EScriptObjectMessage::ToggleActive: return "ToggleActive"sv;
    case EScriptObjectMessage::UNKM18: return "UNKM18"sv;
    case EScriptObjectMessage::Action: return "Action"sv;
    case EScriptObjectMessage::Play: return "Play"sv;
    case EScriptObjectMessage::Alert: return "Alert"sv;
    case EScriptObjectMessage::InternalMessage00: return "InternalMessage00"sv;
    case EScriptObjectMessage::OnFloor: return "OnFloor"sv;
    case EScriptObjectMessage::InternalMessage02: return "InternalMessage02"sv;
    case EScriptObjectMessage::InternalMessage03: return "InternalMessage03"sv;
    case EScriptObjectMessage::Falling: return "Falling"sv;
    case EScriptObjectMessage::OnIceSurface: return "OnIceSurface"sv;
    case EScriptObjectMessage::OnMudSlowSurface: return "OnMudSlowSurface"sv;
    case EScriptObjectMessage::OnNormalSurface: return "OnNormalSurface"sv;
    case EScriptObjectMessage::Touched: return "Touched"sv;
    case EScriptObjectMessage::AddPlatformRider: return "AddPlatformRider"sv;
    case EScriptObjectMessage::LandOnNotFloor: return "LandOnNotFloor"sv;
    case EScriptObjectMessage::Registered: return "Registered"sv;
    case EScriptObjectMessage::Deleted: return "Deleted"sv;
    case EScriptObjectMessage::InitializedInArea: return "InitializedInArea"sv;
    case EScriptObjectMessage::WorldInitialized: return "WorldInitialized"sv;
    case EScriptObjectMessage::AddSplashInhabitant: return "AddSplashInhabitant"sv;
    case EScriptObjectMessage::UpdateSplashInhabitant: return "UpdateSplashInhabitant"sv;
    case EScriptObjectMessage::RemoveSplashInhabitant: return "RemoveSplashInhabitant"sv;
    case EScriptObjectMessage::Jumped: return "Jumped"sv;
    case EScriptObjectMessage::Damage: return "Damage"sv;
    case EScriptObjectMessage::InvulnDamage: return "InvulnDamage"sv;
    case EScriptObjectMessage::ProjectileCollide: return "ProjectileCollide"sv;
    case EScriptObjectMessage::InSnakeWeed: return "InSnakeWeed"sv;
    case EScriptObjectMessage::AddPhazonPoolInhabitant: return "AddPhazonPoolInhabitant"sv;
    case EScriptObjectMessage::UpdatePhazonPoolInhabitant: return "UpdatePhazonPoolInhabitant"sv;
    case EScriptObjectMessage::RemovePhazonPoolInhabitant: return "RemovePhazonPoolInhabitant"sv;
    case EScriptObjectMessage::SuspendedMove: return "SuspendedMove"sv;
    default: return "..."sv;
    }
}
#endif

}
