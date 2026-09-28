#include "Kyoto/Particles/CParticleElectric.hpp" // IWYU pragma: keep
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp" // IWYU pragma: keep
#include "MetroidPrime/CEffect.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CProjectedShadow.hpp" // IWYU pragma: keep
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Cameras/CInterpolationCamera.hpp"
#include "MetroidPrime/Cameras/CPathCamera.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CAmbientAI.hpp"
#include "MetroidPrime/Enemies/CAtomicAlpha.hpp"
#include "MetroidPrime/Enemies/CAtomicBeta.hpp"
#include "MetroidPrime/Enemies/CBabygoth.hpp"
#include "MetroidPrime/Enemies/CBeetle.hpp"
#include "MetroidPrime/Enemies/CBloodFlower.hpp"
#include "MetroidPrime/Enemies/CBouncyGrenade.hpp"
#include "MetroidPrime/Enemies/CBurrower.hpp"
#include "MetroidPrime/Enemies/CChozoGhost.hpp"
#include "MetroidPrime/Enemies/CDrone.hpp"
#include "MetroidPrime/Enemies/CDroneLaser.hpp"
#include "MetroidPrime/Enemies/CElitePirate.hpp"
#include "MetroidPrime/Enemies/CEnergyBall.hpp"
#include "MetroidPrime/Enemies/CEyeBall.hpp"
#include "MetroidPrime/Enemies/CFireFlea.hpp"
#include "MetroidPrime/Enemies/CFlaahgra.hpp"
#include "MetroidPrime/Enemies/CFlaahgraPlants.hpp"
#include "MetroidPrime/Enemies/CFlaahgraTentacle.hpp"
#include "MetroidPrime/Enemies/CFlickerBat.hpp"
#include "MetroidPrime/Enemies/CFlyingPirate.hpp"
#include "MetroidPrime/Enemies/CFlyingPirateRagDoll.hpp" // IWYU pragma: keep
#include "MetroidPrime/Enemies/CGrenadeLauncher.hpp"
#include "MetroidPrime/Enemies/CIceSheegoth.hpp"
#include "MetroidPrime/Enemies/CJellyZap.hpp"
#include "MetroidPrime/Enemies/CMagdolite.hpp"
#include "MetroidPrime/Enemies/CMetaree.hpp"
#include "MetroidPrime/Enemies/CMetroid.hpp"
#include "MetroidPrime/Enemies/CMetroidBeta.hpp"
#include "MetroidPrime/Enemies/CMetroidPrime.hpp"
#include "MetroidPrime/Enemies/CMetroidPrimeRelay.hpp"
#include "MetroidPrime/Enemies/CMetroidPrimeStage2.hpp"
#include "MetroidPrime/Enemies/CNewIntroBoss.hpp"
#include "MetroidPrime/Enemies/COmegaPirate.hpp"
#include "MetroidPrime/Enemies/CParasite.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CPhazonHealingNodule.hpp"
#include "MetroidPrime/Enemies/CPoisonProjectile.hpp"
#include "MetroidPrime/Enemies/CPuddleSpore.hpp"
#include "MetroidPrime/Enemies/CPuddleToadGamma.hpp"
#include "MetroidPrime/Enemies/CPuffer.hpp"
#include "MetroidPrime/Enemies/CRidley.hpp"
#include "MetroidPrime/Enemies/CRipper.hpp"
#include "MetroidPrime/Enemies/CRipperControlledPlatform.hpp"
#include "MetroidPrime/Enemies/CScriptContraption.hpp"
#include "MetroidPrime/Enemies/CScriptPhazonPool.hpp"
#include "MetroidPrime/Enemies/CSeedling.hpp"
#include "MetroidPrime/Enemies/CSpacePirate.hpp"
#include "MetroidPrime/Enemies/CSpankWeed.hpp"
#include "MetroidPrime/Enemies/CTeamAiMgr.hpp"
#include "MetroidPrime/Enemies/CThardus.hpp"
#include "MetroidPrime/Enemies/CThardusRockProjectile.hpp"
#include "MetroidPrime/Enemies/CTryclops.hpp"
#include "MetroidPrime/Enemies/CWallCrawlerSwarm.hpp"
#include "MetroidPrime/Enemies/CWallWalker.hpp"
#include "MetroidPrime/Enemies/CWarWasp.hpp"
#include "MetroidPrime/ScriptObjects/CFire.hpp"
#include "MetroidPrime/ScriptObjects/CFishCloud.hpp"
#include "MetroidPrime/ScriptObjects/CFishCloudModifier.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/ScriptObjects/CRepulsor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActorKeyframe.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActorRotate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAiJumpPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAreaAttributes.hpp"
#include "MetroidPrime/ScriptObjects/CScriptBallTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptBeam.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraBlurKeyframe.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraFilterKeyframe.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraHintTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraPitchVolume.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraShaker.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptColorModulate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptControllerAction.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCounter.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDamageableTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDebris.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDebugCameraWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDistanceFog.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDock.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDockAreaChange.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEMPulse.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGenerator.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGrapplePoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGunTurret.hpp"
#include "MetroidPrime/ScriptObjects/CScriptHUDMemo.hpp"
#include "MetroidPrime/ScriptObjects/CScriptMazeNode.hpp"
#include "MetroidPrime/ScriptObjects/CScriptMemoryRelay.hpp"
#include "MetroidPrime/ScriptObjects/CScriptMidi.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPickup.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPickupGenerator.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerActor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerStateChange.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPointOfInterest.hpp"
#include "MetroidPrime/ScriptObjects/CScriptProjectedShadow.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRandomRelay.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRelay.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRipple.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRoomAcoustics.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSound.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpawnPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpecialFunction.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpiderBallAttractionSurface.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpiderBallWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpindleCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSteam.hpp"
#include "MetroidPrime/ScriptObjects/CScriptStreamedMusic.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSwitch.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTargetingPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTimer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptVisorFlare.hpp"
#include "MetroidPrime/ScriptObjects/CScriptVisorGoo.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWorldTeleporter.hpp"
#include "MetroidPrime/ScriptObjects/CSnakeWeedSwarm.hpp"
#include "MetroidPrime/ScriptObjects/CSustainedPlayerDamage.hpp"
#include "MetroidPrime/Weapons/CBeamProjectile.hpp"
#include "MetroidPrime/Weapons/CBomb.hpp"
#include "MetroidPrime/Weapons/CElectricBeamProjectile.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CFlameThrower.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CIceAttackProjectile.hpp"
#include "MetroidPrime/Weapons/CIceImpact.hpp"
#include "MetroidPrime/Weapons/CNewFlameThrower.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "MetroidPrime/Weapons/CPowerBomb.hpp"
#include "MetroidPrime/Weapons/CShockWave.hpp"
#include "MetroidPrime/Weapons/CTargetableProjectile.hpp"
#include "MetroidPrime/Weapons/CWaveBuster.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include <imgui.h>

void CEntity::DrawInspectorPanel() {
  if (ImGui::CollapsingHeader("CEntity")) {
    ImGui::Text("%s", GetDebugName().data());
  }
}

void CActor::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CActor")) {
    ImGui::Text("Test!");
  }
}

void CScriptActorKeyframe::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptActorKeyframe")) {
    ImGui::Text("Test!");
  }
}

void CScriptActorRotate::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptActorRotate")) {
    ImGui::Text("Test!");
  }
}

void CScriptAreaAttributes::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptAreaAttributes")) {
    ImGui::Text("Test!");
  }
}

void CScriptCameraBlurKeyframe::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptCameraBlurKeyframe")) {
    ImGui::Text("Test!");
  }
}

void CFireFlea::CDeathCameraEffect::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CDeathCameraEffect")) {
    ImGui::Text("Test!");
  }
}

void CScriptCameraFilterKeyframe::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptCameraFilterKeyframe")) {
    ImGui::Text("Test!");
  }
}

void CScriptCameraShaker::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptCameraShaker")) {
    ImGui::Text("Test!");
  }
}

void CScriptColorModulate::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptColorModulate")) {
    ImGui::Text("Test!");
  }
}

void CScriptControllerAction::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptControllerAction")) {
    ImGui::Text("Test!");
  }
}

void CScriptCounter::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptCounter")) {
    ImGui::Text("Test!");
  }
}

void CScriptDistanceFog::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptDistanceFog")) {
    ImGui::Text("Test!");
  }
}

void CScriptDockAreaChange::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptDockAreaChange")) {
    ImGui::Text("Test!");
  }
}

void CMetroidPrimeRelay::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CMetroidPrimeRelay")) {
    ImGui::Text("Test!");
  }
}

void CScriptGenerator::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptGenerator")) {
    ImGui::Text("Test!");
  }
}

void CScriptHUDMemo::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptHUDMemo")) {
    ImGui::Text("Test!");
  }
}

void CScriptMemoryRelay::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptMemoryRelay")) {
    ImGui::Text("Test!");
  }
}

void CScriptMidi::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptMidi")) {
    ImGui::Text("Test!");
  }
}

void CScriptPickupGenerator::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptPickupGenerator")) {
    ImGui::Text("Test!");
  }
}

void CScriptPlayerStateChange::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptPlayerStateChange")) {
    ImGui::Text("Test!");
  }
}

void CScriptRandomRelay::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptRandomRelay")) {
    ImGui::Text("Test!");
  }
}

void CScriptRelay::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptRelay")) {
    ImGui::Text("Test!");
  }
}

void CScriptRipple::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptRipple")) {
    ImGui::Text("Test!");
  }
}

void CScriptRoomAcoustics::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptRoomAcoustics")) {
    ImGui::Text("Test!");
  }
}

void CScriptSpawnPoint::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptSpawnPoint")) {
    ImGui::Text("Test!");
  }
}

void CTeamAiMgr::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CTeamAiMgr")) {
    ImGui::Text("Test!");
  }
}

void CScriptStreamedMusic::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptStreamedMusic")) {
    ImGui::Text("Test!");
  }
}

void CScriptSwitch::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptSwitch")) {
    ImGui::Text("Test!");
  }
}

void CScriptTimer::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptTimer")) {
    ImGui::Text("Test!");
  }
}

void CScriptWorldTeleporter::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptWorldTeleporter")) {
    ImGui::Text("Test!");
  }
}

void CSustainedPlayerDamage::DrawInspectorPanel() {
  CEntity::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CSustainedPlayerDamage")) {
    ImGui::Text("Test!");
  }
}

void CGameCamera::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CGameCamera")) {
    ImGui::Text("Test!");
  }
}

void CEffect::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CEffect")) {
    ImGui::Text("Test!");
  }
}

void CGameLight::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CGameLight")) {
    ImGui::Text("Test!");
  }
}

void CPhysicsActor::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CPhysicsActor")) {
    ImGui::Text("Test!");
  }
}

void CFire::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CFire")) {
    ImGui::Text("Test!");
  }
}

void CFishCloud::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CFishCloud")) {
    ImGui::Text("Test!");
  }
}

void CFishCloudModifier::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CFishCloudModifier")) {
    ImGui::Text("Test!");
  }
}

void CRepulsor::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CRepulsor")) {
    ImGui::Text("Test!");
  }
}

void CScriptAiJumpPoint::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptAiJumpPoint")) {
    ImGui::Text("Test!");
  }
}

void CDroneLaser::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CDroneLaser")) {
    ImGui::Text("Test!");
  }
}

void CScriptBeam::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptBeam")) {
    ImGui::Text("Test!");
  }
}

void CScriptCameraHint::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptCameraHint")) {
    ImGui::Text("Test!");
  }
}

void CFlaahgraRenderer::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CFlaahgraRenderer")) {
    ImGui::Text("Test!");
  }
}

void CScriptCameraHintTrigger::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptCameraHintTrigger")) {
    ImGui::Text("Test!");
  }
}

void CIceAttackProjectile::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CIceAttackProjectile")) {
    ImGui::Text("Test!");
  }
}

void CFlaahgraPlants::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CFlaahgraPlants")) {
    ImGui::Text("Test!");
  }
}

void CScriptCameraPitchVolume::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptCameraPitchVolume")) {
    ImGui::Text("Test!");
  }
}

void CScriptCameraWaypoint::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptCameraWaypoint")) {
    ImGui::Text("Test!");
  }
}

void CScriptCoverPoint::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptCoverPoint")) {
    ImGui::Text("Test!");
  }
}

void CScriptDamageableTrigger::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptDamageableTrigger")) {
    ImGui::Text("Test!");
  }
}

void CScriptDebugCameraWaypoint::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptDebugCameraWaypoint")) {
    ImGui::Text("Test!");
  }
}

void CShockWave::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CShockWave")) {
    ImGui::Text("Test!");
  }
}

void CScriptEffect::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptEffect")) {
    ImGui::Text("Test!");
  }
}

void CWeapon::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CWeapon")) {
    ImGui::Text("Test!");
  }
}

void CScriptEMPulse::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptEMPulse")) {
    ImGui::Text("Test!");
  }
}

void CScriptGrapplePoint::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptGrapplePoint")) {
    ImGui::Text("Test!");
  }
}

void COmegaPirate::CFlash::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CFlash")) {
    ImGui::Text("Test!");
  }
}

void CScriptMazeNode::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptMazeNode")) {
    ImGui::Text("Test!");
  }
}

void CScriptPlayerHint::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptPlayerHint")) {
    ImGui::Text("Test!");
  }
}

void CScriptPointOfInterest::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptPointOfInterest")) {
    ImGui::Text("Test!");
  }
}

void CScriptShadowProjector::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptShadowProjector")) {
    ImGui::Text("Test!");
  }
}

void CScriptSound::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptSound")) {
    ImGui::Text("Test!");
  }
}

void CScriptSpecialFunction::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptSpecialFunction")) {
    ImGui::Text("Test!");
  }
}

void CScriptSpiderBallAttractionSurface::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptSpiderBallAttractionSurface")) {
    ImGui::Text("Test!");
  }
}

void CScriptSpiderBallWaypoint::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptSpiderBallWaypoint")) {
    ImGui::Text("Test!");
  }
}

void CWallCrawlerSwarm::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CWallCrawlerSwarm")) {
    ImGui::Text("Test!");
  }
}

void CScriptTargetingPoint::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptTargetingPoint")) {
    ImGui::Text("Test!");
  }
}

void CScriptTrigger::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptTrigger")) {
    ImGui::Text("Test!");
  }
}

void CScriptVisorFlare::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptVisorFlare")) {
    ImGui::Text("Test!");
  }
}

void CScriptVisorGoo::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptVisorGoo")) {
    ImGui::Text("Test!");
  }
}

void CScriptWaypoint::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptWaypoint")) {
    ImGui::Text("Test!");
  }
}

void CSnakeWeedSwarm::DrawInspectorPanel() {
  CActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CSnakeWeedSwarm")) {
    ImGui::Text("Test!");
  }
}

void CBallCamera::DrawInspectorPanel() {
  CGameCamera::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CBallCamera")) {
    ImGui::Text("Test!");
  }
}

void CPathCamera::DrawInspectorPanel() {
  CGameCamera::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CPathCamera")) {
    ImGui::Text("Test!");
  }
}

void CInterpolationCamera::DrawInspectorPanel() {
  CGameCamera::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CInterpolationCamera")) {
    ImGui::Text("Test!");
  }
}

void CFirstPersonCamera::DrawInspectorPanel() {
  CGameCamera::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CFirstPersonCamera")) {
    ImGui::Text("Test!");
  }
}

void CCinematicCamera::DrawInspectorPanel() {
  CGameCamera::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CCinematicCamera")) {
    ImGui::Text("Test!");
  }
}

void CScriptSpindleCamera::DrawInspectorPanel() {
  CGameCamera::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptSpindleCamera")) {
    ImGui::Text("Test!");
  }
}

void CExplosion::DrawInspectorPanel() {
  CEffect::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CExplosion")) {
    ImGui::Text("Test!");
  }
}

void CHUDBillboardEffect::DrawInspectorPanel() {
  CEffect::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CHUDBillboardEffect")) {
    ImGui::Text("Test!");
  }
}

void CIceImpact::DrawInspectorPanel() {
  CEffect::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CIceImpact")) {
    ImGui::Text("Test!");
  }
}

void CCollisionActor::DrawInspectorPanel() {
  CPhysicsActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CCollisionActor")) {
    ImGui::Text("Test!");
  }
}

void CAmbientAI::DrawInspectorPanel() {
  CPhysicsActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CAmbientAI")) {
    ImGui::Text("Test!");
  }
}

void CAi::DrawInspectorPanel() {
  CPhysicsActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CAi")) {
    ImGui::Text("Test!");
  }
}

void CBouncyGrenade::DrawInspectorPanel() {
  CPhysicsActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CBouncyGrenade")) {
    ImGui::Text("Test!");
  }
}

void CScriptActor::DrawInspectorPanel() {
  CPhysicsActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptActor")) {
    ImGui::Text("Test!");
  }
}

void CGrenadeLauncher::DrawInspectorPanel() {
  CPhysicsActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CGrenadeLauncher")) {
    ImGui::Text("Test!");
  }
}

void CMetroidPrime::CMissileTarget::DrawInspectorPanel() {
  CPhysicsActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CMissileTarget")) {
    ImGui::Text("Test!");
  }
}

void CScriptDebris::DrawInspectorPanel() {
  CPhysicsActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptDebris")) {
    ImGui::Text("Test!");
  }
}

void CScriptDock::DrawInspectorPanel() {
  CPhysicsActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptDock")) {
    ImGui::Text("Test!");
  }
}

void CScriptDoor::DrawInspectorPanel() {
  CPhysicsActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptDoor")) {
    ImGui::Text("Test!");
  }
}

void CScriptGunTurret::DrawInspectorPanel() {
  CPhysicsActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptGunTurret")) {
    ImGui::Text("Test!");
  }
}

void CScriptPickup::DrawInspectorPanel() {
  CPhysicsActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptPickup")) {
    ImGui::Text("Test!");
  }
}

void CScriptPlatform::DrawInspectorPanel() {
  CPhysicsActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptPlatform")) {
    ImGui::Text("Test!");
  }
}

void CBomb::DrawInspectorPanel() {
  CWeapon::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CBomb")) {
    ImGui::Text("Test!");
  }
}

void CGameProjectile::DrawInspectorPanel() {
  CWeapon::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CGameProjectile")) {
    ImGui::Text("Test!");
  }
}

void CPowerBomb::DrawInspectorPanel() {
  CWeapon::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CPowerBomb")) {
    ImGui::Text("Test!");
  }
}

void CScriptBallTrigger::DrawInspectorPanel() {
  CScriptTrigger::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptBallTrigger")) {
    ImGui::Text("Test!");
  }
}

void CScriptPhazonPool::DrawInspectorPanel() {
  CScriptTrigger::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptPhazonPool")) {
    ImGui::Text("Test!");
  }
}

void CScriptSteam::DrawInspectorPanel() {
  CScriptTrigger::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptSteam")) {
    ImGui::Text("Test!");
  }
}

void CScriptWater::DrawInspectorPanel() {
  CScriptTrigger::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptWater")) {
    ImGui::Text("Test!");
  }
}

void CPatterned::DrawInspectorPanel() {
  CAi::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CPatterned")) {
    ImGui::Text("Test!");
  }
}

void CDestroyableRock::DrawInspectorPanel() {
  CAi::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CDestroyableRock")) {
    ImGui::Text("Test!");
  }
}

void CEnergyProjectile::DrawInspectorPanel() {
  CGameProjectile::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CEnergyProjectile")) {
    ImGui::Text("Test!");
  }
}

void CBeamProjectile::DrawInspectorPanel() {
  CGameProjectile::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CBeamProjectile")) {
    ImGui::Text("Test!");
  }
}

void CNewFlameThrower::DrawInspectorPanel() {
  CGameProjectile::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CNewFlameThrower")) {
    ImGui::Text("Test!");
  }
}

void CWaveBuster::DrawInspectorPanel() {
  CGameProjectile::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CWaveBuster")) {
    ImGui::Text("Test!");
  }
}

void CFlameThrower::DrawInspectorPanel() {
  CGameProjectile::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CFlameThrower")) {
    ImGui::Text("Test!");
  }
}

void CScriptContraption::DrawInspectorPanel() {
  CScriptActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptContraption")) {
    ImGui::Text("Test!");
  }
}

void CScriptPlayerActor::DrawInspectorPanel() {
  CScriptActor::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CScriptPlayerActor")) {
    ImGui::Text("Test!");
  }
}

void CRipperControlledPlatform::DrawInspectorPanel() {
  CScriptPlatform::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CRipperControlledPlatform")) {
    ImGui::Text("Test!");
  }
}

void CAtomicBeta::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CAtomicBeta")) {
    ImGui::Text("Test!");
  }
}

void CAtomicAlpha::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CAtomicAlpha")) {
    ImGui::Text("Test!");
  }
}

void CBloodFlower::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CBloodFlower")) {
    ImGui::Text("Test!");
  }
}

void CBurrower::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CBurrower")) {
    ImGui::Text("Test!");
  }
}

void CBabygoth::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CBabygoth")) {
    ImGui::Text("Test!");
  }
}

void CBeetle::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CBeetle")) {
    ImGui::Text("Test!");
  }
}

void CChozoGhost::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CChozoGhost")) {
    ImGui::Text("Test!");
  }
}

void CDrone::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CDrone")) {
    ImGui::Text("Test!");
  }
}

void CElitePirate::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CElitePirate")) {
    ImGui::Text("Test!");
  }
}

void CFireFlea::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CFireFlea")) {
    ImGui::Text("Test!");
  }
}

void CEyeBall::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CEyeBall")) {
    ImGui::Text("Test!");
  }
}

void CEnergyBall::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CEnergyBall")) {
    ImGui::Text("Test!");
  }
}

void CFlyingPirate::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CFlyingPirate")) {
    ImGui::Text("Test!");
  }
}

void CFlickerBat::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CFlickerBat")) {
    ImGui::Text("Test!");
  }
}

void CFlaahgraTentacle::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CFlaahgraTentacle")) {
    ImGui::Text("Test!");
  }
}

void CFlaahgra::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CFlaahgra")) {
    ImGui::Text("Test!");
  }
}

void CJellyZap::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CJellyZap")) {
    ImGui::Text("Test!");
  }
}

void CIceSheegoth::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CIceSheegoth")) {
    ImGui::Text("Test!");
  }
}

void CMetaree::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CMetaree")) {
    ImGui::Text("Test!");
  }
}

void CMetroidBeta::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CMetroidBeta")) {
    ImGui::Text("Test!");
  }
}

void CMetroid::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CMetroid")) {
    ImGui::Text("Test!");
  }
}

void CMagdolite::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CMagdolite")) {
    ImGui::Text("Test!");
  }
}

void CMetroidPrimeStage2::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CMetroidPrimeStage2")) {
    ImGui::Text("Test!");
  }
}

void CMetroidPrime::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CMetroidPrime")) {
    ImGui::Text("Test!");
  }
}

void CNewIntroBoss::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CNewIntroBoss")) {
    ImGui::Text("Test!");
  }
}

void CPhazonHealingNodule::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CPhazonHealingNodule")) {
    ImGui::Text("Test!");
  }
}

void CPuddleSpore::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CPuddleSpore")) {
    ImGui::Text("Test!");
  }
}

void CPuddleToadGamma::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CPuddleToadGamma")) {
    ImGui::Text("Test!");
  }
}

void CPuffer::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CPuffer")) {
    ImGui::Text("Test!");
  }
}

void CRidley::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CRidley")) {
    ImGui::Text("Test!");
  }
}

void CRipper::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CRipper")) {
    ImGui::Text("Test!");
  }
}

void CSpacePirate::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CSpacePirate")) {
    ImGui::Text("Test!");
  }
}

void CSpankWeed::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CSpankWeed")) {
    ImGui::Text("Test!");
  }
}

void CThardusRockProjectile::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CThardusRockProjectile")) {
    ImGui::Text("Test!");
  }
}

void CTryclops::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CTryclops")) {
    ImGui::Text("Test!");
  }
}

void CThardus::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CThardus")) {
    ImGui::Text("Test!");
  }
}

void CWallWalker::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CWallWalker")) {
    ImGui::Text("Test!");
  }
}

void CWarWasp::DrawInspectorPanel() {
  CPatterned::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CWarWasp")) {
    ImGui::Text("Test!");
  }
}

void CPoisonProjectile::DrawInspectorPanel() {
  CEnergyProjectile::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CPoisonProjectile")) {
    ImGui::Text("Test!");
  }
}

void CTargetableProjectile::DrawInspectorPanel() {
  CEnergyProjectile::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CTargetableProjectile")) {
    ImGui::Text("Test!");
  }
}

void CElectricBeamProjectile::DrawInspectorPanel() {
  CBeamProjectile::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CElectricBeamProjectile")) {
    ImGui::Text("Test!");
  }
}

void CPlasmaProjectile::DrawInspectorPanel() {
  CBeamProjectile::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CPlasmaProjectile")) {
    ImGui::Text("Test!");
  }
}

void COmegaPirate::DrawInspectorPanel() {
  CElitePirate::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("COmegaPirate")) {
    ImGui::Text("Test!");
  }
}

void CParasite::DrawInspectorPanel() {
  CWallWalker::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CParasite")) {
    ImGui::Text("Test!");
  }
}

void CSeedling::DrawInspectorPanel() {
  CWallWalker::DrawInspectorPanel();
  if (ImGui::CollapsingHeader("CSeedling")) {
    ImGui::Text("Test!");
  }
}
