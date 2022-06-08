#include "Runtime/ImGuiEntitySupport.hpp"

#include "Runtime/World/CActor.hpp"
#include "Runtime/World/CAi.hpp"
#include "Runtime/World/CAmbientAI.hpp"
#include "Runtime/World/CDestroyableRock.hpp"
#include "Runtime/World/CEffect.hpp"
#include "Runtime/World/CEntity.hpp"
#include "Runtime/World/CExplosion.hpp"
#include "Runtime/World/CFire.hpp"
#include "Runtime/World/CFishCloud.hpp"
#include "Runtime/World/CFishCloudModifier.hpp"
#include "Runtime/World/CGameLight.hpp"
#include "Runtime/World/CHUDBillboardEffect.hpp"
#include "Runtime/World/CIceImpact.hpp"
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CRepulsor.hpp"
#include "Runtime/World/CScriptActor.hpp"
#include "Runtime/World/CScriptActorKeyframe.hpp"
#include "Runtime/World/CScriptActorRotate.hpp"
#include "Runtime/World/CScriptAiJumpPoint.hpp"
#include "Runtime/World/CScriptAreaAttributes.hpp"
#include "Runtime/World/CScriptBallTrigger.hpp"
#include "Runtime/World/CScriptBeam.hpp"
#include "Runtime/World/CScriptCameraBlurKeyframe.hpp"
#include "Runtime/World/CScriptCameraFilterKeyframe.hpp"
#include "Runtime/World/CScriptCameraHint.hpp"
#include "Runtime/World/CScriptCameraHintTrigger.hpp"
#include "Runtime/World/CScriptCameraPitchVolume.hpp"
#include "Runtime/World/CScriptCameraShaker.hpp"
#include "Runtime/World/CScriptCameraWaypoint.hpp"
#include "Runtime/World/CScriptColorModulate.hpp"
#include "Runtime/World/CScriptControllerAction.hpp"
#include "Runtime/World/CScriptCounter.hpp"
#include "Runtime/World/CScriptCoverPoint.hpp"
#include "Runtime/World/CScriptDamageableTrigger.hpp"
#include "Runtime/World/CScriptDebris.hpp"
#include "Runtime/World/CScriptDebugCameraWaypoint.hpp"
#include "Runtime/World/CScriptDistanceFog.hpp"
#include "Runtime/World/CScriptDock.hpp"
#include "Runtime/World/CScriptDockAreaChange.hpp"
#include "Runtime/World/CScriptDoor.hpp"
#include "Runtime/World/CScriptEMPulse.hpp"
#include "Runtime/World/CScriptEffect.hpp"
#include "Runtime/World/CScriptGenerator.hpp"
#include "Runtime/World/CScriptGrapplePoint.hpp"
#include "Runtime/World/CScriptGunTurret.hpp"
#include "Runtime/World/CScriptHUDMemo.hpp"
#include "Runtime/World/CScriptMazeNode.hpp"
#include "Runtime/World/CScriptMemoryRelay.hpp"
#include "Runtime/World/CScriptMidi.hpp"
#include "Runtime/World/CScriptPickup.hpp"
#include "Runtime/World/CScriptPickupGenerator.hpp"
#include "Runtime/World/CScriptPlatform.hpp"
#include "Runtime/World/CScriptPlayerActor.hpp"
#include "Runtime/World/CScriptPlayerHint.hpp"
#include "Runtime/World/CScriptPlayerStateChange.hpp"
#include "Runtime/World/CScriptPointOfInterest.hpp"
#include "Runtime/World/CScriptRandomRelay.hpp"
#include "Runtime/World/CScriptRelay.hpp"
#include "Runtime/World/CScriptRipple.hpp"
#include "Runtime/World/CScriptRoomAcoustics.hpp"
#include "Runtime/World/CScriptShadowProjector.hpp"
#include "Runtime/World/CScriptSound.hpp"
#include "Runtime/World/CScriptSpawnPoint.hpp"
#include "Runtime/World/CScriptSpecialFunction.hpp"
#include "Runtime/World/CScriptSpiderBallAttractionSurface.hpp"
#include "Runtime/World/CScriptSpiderBallWaypoint.hpp"
#include "Runtime/World/CScriptSpindleCamera.hpp"
#include "Runtime/World/CScriptSteam.hpp"
#include "Runtime/World/CScriptStreamedMusic.hpp"
#include "Runtime/World/CScriptSwitch.hpp"
#include "Runtime/World/CScriptTargetingPoint.hpp"
#include "Runtime/World/CScriptTimer.hpp"
#include "Runtime/World/CScriptTrigger.hpp"
#include "Runtime/World/CScriptVisorFlare.hpp"
#include "Runtime/World/CScriptVisorGoo.hpp"
#include "Runtime/World/CScriptWater.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "Runtime/World/CScriptWorldTeleporter.hpp"
#include "Runtime/World/CSnakeWeedSwarm.hpp"
#include "Runtime/World/CTeamAiMgr.hpp"
#include "Runtime/World/CWallCrawlerSwarm.hpp"
#include "Runtime/World/CWallWalker.hpp"

#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/Camera/CCinematicCamera.hpp"
#include "Runtime/Camera/CFirstPersonCamera.hpp"
#include "Runtime/Camera/CInterpolationCamera.hpp"
#include "Runtime/Camera/CPathCamera.hpp"

#include "Runtime/Collision/CCollisionActor.hpp"

#include "Runtime/Weapon/CWeapon.hpp"
#include "Runtime/Weapon/CBeamProjectile.hpp"
#include "Runtime/Weapon/CBomb.hpp"
#include "Runtime/Weapon/CElectricBeamProjectile.hpp"
#include "Runtime/Weapon/CFlameThrower.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/Weapon/CNewFlameThrower.hpp"
#include "Runtime/Weapon/CPlasmaProjectile.hpp"
#include "Runtime/Weapon/CPowerBomb.hpp"
#include "Runtime/Weapon/CTargetableProjectile.hpp"
#include "Runtime/Weapon/CWaveBuster.hpp"

#include "Runtime/MP1/World/CActorContraption.hpp"
#include "Runtime/MP1/World/CAtomicAlpha.hpp"
#include "Runtime/MP1/World/CAtomicBeta.hpp"
#include "Runtime/MP1/World/CBabygoth.hpp"
#include "Runtime/MP1/World/CBeetle.hpp"
#include "Runtime/MP1/World/CBloodFlower.hpp"
#include "Runtime/MP1/World/CBouncyGrenade.hpp"
#include "Runtime/MP1/World/CBurrower.hpp"
#include "Runtime/MP1/World/CChozoGhost.hpp"
#include "Runtime/MP1/World/CDrone.hpp"
#include "Runtime/MP1/World/CDroneLaser.hpp"
#include "Runtime/MP1/World/CElitePirate.hpp"
#include "Runtime/MP1/World/CEnergyBall.hpp"
#include "Runtime/MP1/World/CEyeball.hpp"
#include "Runtime/MP1/World/CFireFlea.hpp"
#include "Runtime/MP1/World/CFlaahgra.hpp"
#include "Runtime/MP1/World/CFlaahgraProjectile.hpp"
#include "Runtime/MP1/World/CFlaahgraTentacle.hpp"
#include "Runtime/MP1/World/CFlickerBat.hpp"
#include "Runtime/MP1/World/CFlyingPirate.hpp"
#include "Runtime/MP1/World/CGrenadeLauncher.hpp"
#include "Runtime/MP1/World/CIceAttackProjectile.hpp"
#include "Runtime/MP1/World/CIceSheegoth.hpp"
#include "Runtime/MP1/World/CJellyZap.hpp"
#include "Runtime/MP1/World/CMagdolite.hpp"
#include "Runtime/MP1/World/CMetaree.hpp"
#include "Runtime/MP1/World/CMetroid.hpp"
#include "Runtime/MP1/World/CMetroidBeta.hpp"
#include "Runtime/MP1/World/CMetroidPrimeEssence.hpp"
#include "Runtime/MP1/World/CMetroidPrimeExo.hpp"
#include "Runtime/MP1/World/CMetroidPrimeProjectile.hpp"
#include "Runtime/MP1/World/CMetroidPrimeRelay.hpp"
#include "Runtime/MP1/World/CNewIntroBoss.hpp"
#include "Runtime/MP1/World/COmegaPirate.hpp"
#include "Runtime/MP1/World/CParasite.hpp"
#include "Runtime/MP1/World/CPhazonHealingNodule.hpp"
#include "Runtime/MP1/World/CPhazonPool.hpp"
#include "Runtime/MP1/World/CPuddleSpore.hpp"
#include "Runtime/MP1/World/CPuddleToadGamma.hpp"
#include "Runtime/MP1/World/CPuffer.hpp"
#include "Runtime/MP1/World/CRidley.hpp"
#include "Runtime/MP1/World/CRipper.hpp"
#include "Runtime/MP1/World/CSeedling.hpp"
#include "Runtime/MP1/World/CShockWave.hpp"
#include "Runtime/MP1/World/CSpacePirate.hpp"
#include "Runtime/MP1/World/CSpankWeed.hpp"
#include "Runtime/MP1/World/CThardus.hpp"
#include "Runtime/MP1/World/CThardusRockProjectile.hpp"
#include "Runtime/MP1/World/CTryclops.hpp"
#include "Runtime/MP1/World/CWarWasp.hpp"

#include "Runtime/World/CWorld.hpp"
#include "Runtime/World/CGameArea.hpp"
#include "Runtime/GameGlobalObjects.hpp"

#include "ImGuiConsole.hpp"
#include "imgui.h"
#include "magic_enum.hpp"

#define IMGUI_ENTITY_INSPECT(CLS, PARENT, NAME, ...)                                                                   \
  std::string_view CLS::ImGuiType() { return #NAME; }                                                                  \
  void CLS::ImGuiInspect() {                                                                                           \
    PARENT::ImGuiInspect();                                                                                            \
    if (ImGui::CollapsingHeader(#NAME))                                                                                \
      __VA_ARGS__                                                                                                      \
  }

#define BITFIELD_CHECKBOX(label, bf, ...)                                                                              \
  {                                                                                                                    \
    bool b = (bf);                                                                                                     \
    if (ImGui::Checkbox(label, &b)) {                                                                                  \
      (bf) = b;                                                                                                        \
      __VA_ARGS__                                                                                                      \
    }                                                                                                                  \
  }

namespace metaforce {

bool ImGuiVector3fInput(const char* label, zeus::CVector3f& vec) {
  std::array<float, 3> arr{vec.x(), vec.y(), vec.z()};
  if (ImGui::DragFloat3(label, arr.data(), 0.1f)) {
    vec.assign(arr[0], arr[1], arr[2]);
    return true;
  }
  return false;
}

static bool ImGuiColorInput(const char* label, zeus::CColor& col) {
  std::array<float, 4> arr{col.r(), col.g(), col.b(), col.a()};
  if (ImGui::ColorEdit4(label, arr.data())) {
    col = zeus::CColor{arr[0], arr[1], arr[2], arr[3]};
    return true;
  }
  return false;
}

template <typename E>
static constexpr bool ImGuiEnumInput(const char* label, E& val) {
  constexpr auto& entries = magic_enum::enum_entries<E>();
  bool changed = false;
  if (ImGui::BeginCombo(label, magic_enum::enum_name(val).data())) {
    for (const auto& [item, name] : entries) {
      if (ImGui::Selectable(name.data(), item == val)) {
        val = item;
        changed = true;
      }
    }
    ImGui::EndCombo();
  }
  return changed;
}

void ImGuiAnimRes(const char* label, metaforce::CAnimRes& res) {
  if (res.GetId().IsValid()) {
    ImGui::Text("Model: 0x%08" PRIX64, res.GetId().Value());
  } else {
    ImGui::Text("Model: [none]");
  }
  // TODO: More
}

void CDamageVulnerability::ImGuiEditWindow(const char* title, bool& open) {
  if (!open) {
    return;
  }
  if (ImGui::Begin(title, &open, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGuiEnumInput("Deflected", x5c_deflected);
    if (ImGui::CollapsingHeader("Normal")) {
      constexpr size_t max = std::tuple_size_v<decltype(x0_normal)>;
      constexpr std::array<const char*, max> names{
          "Power",  "Ice", "Wave",         "Plasma", "Bomb", "Power Bomb", "Missile",      "Boost Ball",
          "Phazon", "AI",  "Poison Water", "Lava",   "Heat", "(Unused)",   "Orange Phazon"};
      for (int i = 0; i < max; ++i) {
        ImGuiEnumInput(names[i], x0_normal[i]);
      }
    }
    if (ImGui::CollapsingHeader("Charged")) {
      constexpr size_t max = std::tuple_size_v<decltype(x3c_charged)>;
      constexpr std::array<const char*, max> names{"Power", "Ice", "Wave", "Plasma"};
      for (int i = 0; i < max; ++i) {
        ImGuiEnumInput(names[i], x3c_charged[i]);
      }
    }
    if (ImGui::CollapsingHeader("Combo")) {
      constexpr size_t max = std::tuple_size_v<decltype(x4c_combo)>;
      constexpr std::array<const char*, max> names{"Super Missile", "Ice Spreader", "Wavebuster", "Flamethrower"};
      for (int i = 0; i < max; ++i) {
        ImGuiEnumInput(names[i], x4c_combo[i]);
      }
    }
  }
  ImGui::End();
}

void ImGuiUniqueId(const char* label, TUniqueId uid) {
  ImGui::PushID(uid.Value());
  if (uid != kInvalidUniqueId && ImGuiConsole::entities[uid.Value()].ent != nullptr) {
    ImGui::Text("%s: 0x%04" PRIX16, label, uid.Value());
    ImGui::SameLine();
    if (ImGui::SmallButton("View")) {
      ImGuiConsole::inspectingEntities.insert(uid);
    }
  } else {
    ImGui::Text("%s: [none]", label);
  }
  ImGui::PopID();
}

std::string_view CEntity::ImGuiType() { return "Entity"; }

void CEntity::ImGuiInspect() {
  if (!x20_conns.empty() && ImGui::CollapsingHeader("Outgoing Connections")) {
    if (ImGui::BeginTable("Outgoing Connections", 6,
                          ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV)) {
      ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 0, 'id');
      ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 0, 'type');
      ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0, 'name');
      ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 0, 'stat');
      ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthFixed, 0, 'msg');
      ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed |
                                      ImGuiTableColumnFlags_NoResize);
      ImGui::TableSetupScrollFreeze(0, 1);
      ImGui::TableHeadersRow();
      for (const auto& item : x20_conns) {
        const auto search = g_StateManager->GetIdListForScript(item.x8_objId);
        for (auto it = search.first; it != search.second; ++it) {
          const auto uid = it->second;
          if (uid == kInvalidUniqueId) {
            continue;
          }
          ImGuiEntityEntry& entry = ImGuiConsole::entities[uid.Value()];
          if (entry.uid == kInvalidUniqueId) {
            continue;
          }
          ImGuiConsole::BeginEntityRow(entry);
          if (ImGui::TableNextColumn()) {
            ImGuiStringViewText(entry.type);
          }
          if (ImGui::TableNextColumn()) {
            ImGuiStringViewText(entry.name);
          }
          if (ImGui::TableNextColumn()) {
            ImGuiStringViewText(ScriptObjectStateToStr(item.x0_state));
          }
          if (ImGui::TableNextColumn()) {
            ImGuiStringViewText(ScriptObjectMessageToStr(item.x4_msg));
          }
          ImGuiConsole::EndEntityRow(entry);
        }
      }
      ImGui::EndTable();
    }
  }
  if (m_incomingConnections != nullptr && ImGui::CollapsingHeader("Incoming Connections")) {
    if (ImGui::BeginTable("Incoming Connections", 6,
                          ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV)) {
      ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 0, 'id');
      ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 0, 'type');
      ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0, 'name');
      ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 0, 'stat');
      ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthFixed, 0, 'msg');
      ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed |
                                      ImGuiTableColumnFlags_NoResize);
      ImGui::TableSetupScrollFreeze(0, 1);
      ImGui::TableHeadersRow();
      for (const auto& item : *m_incomingConnections) {
        const auto search = g_StateManager->GetIdListForScript(item.x8_objId);
        for (auto it = search.first; it != search.second; ++it) {
          auto uid = it->second;
          if (uid == kInvalidUniqueId) {
            continue;
          }
          ImGuiEntityEntry& entry = ImGuiConsole::entities[uid.Value()];
          if (entry.uid == kInvalidUniqueId) {
            continue;
          }
          ImGuiConsole::BeginEntityRow(entry);
          if (ImGui::TableNextColumn()) {
            ImGuiStringViewText(entry.type);
          }
          if (ImGui::TableNextColumn()) {
            ImGuiStringViewText(entry.name);
          }
          if (ImGui::TableNextColumn()) {
            ImGuiStringViewText(ScriptObjectStateToStr(item.x0_state));
          }
          if (ImGui::TableNextColumn()) {
            ImGuiStringViewText(ScriptObjectMessageToStr(item.x4_msg));
          }
          ImGuiConsole::EndEntityRow(entry);
        }
      }
      ImGui::EndTable();
    }
  }
  if (ImGui::CollapsingHeader("Entity", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("ID: 0x%04" PRIX16, x8_uid.Value());
    ImGui::Text("Editor ID: 0x%08" PRIX32, xc_editorId.id);
    ImGui::Text("Area: %i", x4_areaId);
    ImGui::Text("Name: %s", x10_name.c_str());
    BITFIELD_CHECKBOX("Active", x30_24_active);
    ImGui::SameLine();
    ImGui::Checkbox("Highlight", &m_debugSelected);
  }
}

struct EulerAngles {
  float roll, pitch, yaw;
};

static EulerAngles ToEulerAngles(const zeus::CQuaternion& q) {
  EulerAngles angles;

  // roll (x-axis rotation)
  float sinr_cosp = 2.f * (q.w() * q.x() + q.y() * q.z());
  float cosr_cosp = 1.f - 2.f * (q.x() * q.x() + q.y() * q.y());
  angles.roll = atan2f(sinr_cosp, cosr_cosp);

  // pitch (y-axis rotation)
  float sinp = 2.f * (q.w() * q.y() - q.z() * q.x());
  if (std::abs(sinp) >= 1.f) {
    angles.pitch = std::copysign(M_PI / 2, sinp); // use 90 degrees if out of range
  } else {
    angles.pitch = std::asin(sinp);
  }

  // yaw (z-axis rotation)
  float siny_cosp = 2.f * (q.w() * q.z() + q.x() * q.y());
  float cosy_cosp = 1.f - 2.f * (q.y() * q.y() + q.z() * q.z());
  angles.yaw = atan2f(siny_cosp, cosy_cosp);

  return angles;
}

// <- CEntity
IMGUI_ENTITY_INSPECT(CActor, CEntity, Actor, {
  if (ImGuiVector3fInput("Position", x34_transform.origin)) {
    SetTranslation(x34_transform.origin);
  }
  EulerAngles angles = ToEulerAngles(zeus::CQuaternion(GetTransform().getRotation().buildMatrix3f()));
  zeus::CVector3f rotation = zeus::CVector3f(angles.roll, angles.pitch, angles.yaw) * zeus::skRadToDegVec;
  if (ImGuiVector3fInput("Rotation", rotation)) {
    rotation.x() = zeus::clamp(-179.999f, float(rotation.x()), 179.999f);
    rotation.y() = zeus::clamp(-89.999f, float(rotation.y()), 89.999f);
    rotation.z() = zeus::clamp(-179.999f, float(rotation.z()), 179.999f);
    x34_transform.setRotation(zeus::CQuaternion(rotation * zeus::skDegToRadVec).toTransform().buildMatrix3f());
    SetTransform(x34_transform);
  }
  {
    int thermalVisorFlags = xe6_27_thermalVisorFlags;
    if (ImGui::Combo("Thermal Visor Flags", &thermalVisorFlags, "None\0Cold\0Hot", 3)) {
      xe6_27_thermalVisorFlags = u8(thermalVisorFlags);
    }
  }
})
IMGUI_ENTITY_INSPECT(MP1::CFireFlea::CDeathCameraEffect, CEntity, FireFleaDeathCameraEffect, {})
IMGUI_ENTITY_INSPECT(MP1::CMetroidPrimeRelay, CEntity, MetroidPrimeRelay, {})
IMGUI_ENTITY_INSPECT(CScriptActorKeyframe, CEntity, ScriptActorKeyframe, {})
IMGUI_ENTITY_INSPECT(CScriptActorRotate, CEntity, ScriptActorRotate, {})

IMGUI_ENTITY_INSPECT(CScriptAreaAttributes, CEntity, ScriptAreaAttributes, {
  BITFIELD_CHECKBOX("Show Skybox", x34_24_showSkybox);
  ImGui::SameLine();
  ImGui::Text("(Asset: 0x%08X)", int(x4c_skybox.Value()));
  ImGuiEnumInput("Env FX Type", x38_envFx);
  if (ImGui::SliderFloat("Env FX Density", &x3c_envFxDensity, 0.f, 1.f)) {
    g_StateManager->GetEnvFxManager()->SetFxDensity(500, x3c_envFxDensity);
  }
  if (ImGui::SliderFloat("Thermal Heat", &x40_thermalHeat, 0.f, 1.f)) {
    CGameArea* area = g_StateManager->GetWorld()->GetArea(x4_areaId);
    if (area != nullptr && area->IsPostConstructed()) {
      area->GetPostConstructed()->x111c_thermalCurrent = x40_thermalHeat;
    }
  }
  ImGui::SliderFloat("X-Ray Fog Distance", &x44_xrayFogDistance, 0.f, 1.f);
  if (ImGui::SliderFloat("World Lighting Level", &x48_worldLightingLevel, 0.f, 1.f)) {
    CGameArea* area = g_StateManager->GetWorld()->GetArea(x4_areaId);
    if (area != nullptr && area->IsPostConstructed()) {
      area->GetPostConstructed()->x1128_worldLightingLevel = x48_worldLightingLevel;
    }
  }
  ImGuiEnumInput("Phazon Type", x50_phazon);
})
IMGUI_ENTITY_INSPECT(CScriptCameraBlurKeyframe, CEntity, ScriptCameraBlurKeyframe, {})
IMGUI_ENTITY_INSPECT(CScriptCameraFilterKeyframe, CEntity, ScriptCameraFilterKeyframe, {})
IMGUI_ENTITY_INSPECT(CScriptCameraShaker, CEntity, ScriptCameraShaker, {})
IMGUI_ENTITY_INSPECT(CScriptColorModulate, CEntity, ScriptColorModulate, {})
IMGUI_ENTITY_INSPECT(CScriptControllerAction, CEntity, ScriptControllerAction, {})
IMGUI_ENTITY_INSPECT(CScriptCounter, CEntity, ScriptCounter, {})
IMGUI_ENTITY_INSPECT(CScriptDistanceFog, CEntity, ScriptDistanceFog, {})
IMGUI_ENTITY_INSPECT(CScriptDockAreaChange, CEntity, ScriptDockAreaChange, {})
IMGUI_ENTITY_INSPECT(CScriptGenerator, CEntity, ScriptGenerator, {
  int count = x34_spawnCount;
  if (ImGui::SliderInt("Spawn Count", &count, 0, kMaxEntities)) {
    x34_spawnCount = count;
  }
  BITFIELD_CHECKBOX("Don't Reuse Followers", x38_24_noReuseFollowers);
  BITFIELD_CHECKBOX("Don't Inherit Transform", x38_25_noInheritTransform);
  ImGuiVector3fInput("Offset", x3c_offset);
  ImGui::DragFloat("Minimum Scale", &x48_minScale);
  ImGui::DragFloat("Maximum Scale", &x4c_maxScale);
})
IMGUI_ENTITY_INSPECT(CScriptHUDMemo, CEntity, ScriptHUDMemo, {})
IMGUI_ENTITY_INSPECT(CScriptMemoryRelay, CEntity, ScriptMemoryRelay, {})
IMGUI_ENTITY_INSPECT(CScriptMidi, CEntity, ScriptMidi, {})
IMGUI_ENTITY_INSPECT(CScriptPickupGenerator, CEntity, ScriptPickupGenerator, {
  ImGuiVector3fInput("Position", x34_position);
  ImGui::DragFloat("Frequency", &x40_frequency);
  ImGui::DragFloat("Delay Timer", &x44_delayTimer);
})
IMGUI_ENTITY_INSPECT(CScriptPlayerStateChange, CEntity, ScriptPlayerStateChange, {})
IMGUI_ENTITY_INSPECT(CScriptRandomRelay, CEntity, ScriptRandomRelay, {})
IMGUI_ENTITY_INSPECT(CScriptRelay, CEntity, ScriptRelay, {})
IMGUI_ENTITY_INSPECT(CScriptRipple, CEntity, ScripleRipple, {})
IMGUI_ENTITY_INSPECT(CScriptRoomAcoustics, CEntity, ScriptRoomAcoustics, {})
IMGUI_ENTITY_INSPECT(CScriptSpawnPoint, CEntity, ScriptSpawnPoint, {})
IMGUI_ENTITY_INSPECT(CScriptStreamedMusic, CEntity, ScriptStreamedMusic, {})
IMGUI_ENTITY_INSPECT(CScriptSwitch, CEntity, ScriptSwitch, {
  ImGui::Checkbox("Is Open", &x34_opened);
  ImGui::Checkbox("Close On Opened", &x35_closeOnOpened);
  if (ImGui::Button("Open")) {
    g_StateManager->SendScriptMsg(this, x8_uid, EScriptObjectMessage::Open);
  }
  ImGui::SameLine();
  if (ImGui::Button("Close")) {
    g_StateManager->SendScriptMsg(this, x8_uid, EScriptObjectMessage::Close);
  }
  ImGui::SameLine();
  if (ImGui::Button("Toggle")) {
    g_StateManager->SendScriptMsg(this, x8_uid, EScriptObjectMessage::SetToZero);
  }
})

IMGUI_ENTITY_INSPECT(CScriptTimer, CEntity, ScriptTimer, {
  ImGui::DragFloat("Time", &x34_time);
  ImGui::DragFloat("Start Time", &x38_startTime);
  ImGui::DragFloat("Max Random Delay", &x3c_maxRandDelay);
  ImGui::Checkbox("Loop", &x40_loop);
  ImGui::Checkbox("Auto Start", &x41_autoStart);
  ImGui::Checkbox("Is Timing", &x42_isTiming);
  if (ImGui::Button("Reset")) {
    g_StateManager->SendScriptMsg(this, x8_uid, EScriptObjectMessage::ResetAndStart);
  }
})
IMGUI_ENTITY_INSPECT(CScriptWorldTeleporter, CEntity, ScriptWorldTeleporter, {})
IMGUI_ENTITY_INSPECT(CTeamAiMgr, CEntity, TeamAiMgr, {
  // TODO x34_data
  // TODO x58_roles
  if (!x68_meleeAttackers.empty() && ImGui::CollapsingHeader("Melee Attackers")) {
    if (ImGui::BeginTable("Melee Attackers", 4,
                          ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV)) {
      ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 0, 'id');
      ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 0, 'type');
      ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0, 'name');
      ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed |
                                      ImGuiTableColumnFlags_NoResize);
      ImGui::TableSetupScrollFreeze(0, 1);
      ImGui::TableHeadersRow();
      for (auto uid : x68_meleeAttackers) {
        if (uid == kInvalidUniqueId) {
          continue;
        }
        ImGuiEntityEntry& entry = ImGuiConsole::entities[uid.Value()];
        if (entry.uid == kInvalidUniqueId) {
          continue;
        }
        ImGuiConsole::BeginEntityRow(entry);
        if (ImGui::TableNextColumn()) {
          ImGuiStringViewText(entry.type);
        }
        if (ImGui::TableNextColumn()) {
          ImGuiStringViewText(entry.name);
        }
        ImGuiConsole::EndEntityRow(entry);
      }
    }
    ImGui::EndTable();
  }
  if (!x78_rangedAttackers.empty() && ImGui::CollapsingHeader("Ranged Attackers")) {
    if (ImGui::BeginTable("Ranged Attackers", 4,
                          ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV)) {
      ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 0, 'id');
      ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 0, 'type');
      ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0, 'name');
      ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed |
                                      ImGuiTableColumnFlags_NoResize);
      ImGui::TableSetupScrollFreeze(0, 1);
      ImGui::TableHeadersRow();
      for (auto uid : x78_rangedAttackers) {
        if (uid == kInvalidUniqueId) {
          continue;
        }
        ImGuiEntityEntry& entry = ImGuiConsole::entities[uid.Value()];
        if (entry.uid == kInvalidUniqueId) {
          continue;
        }
        ImGuiConsole::BeginEntityRow(entry);
        if (ImGui::TableNextColumn()) {
          ImGuiStringViewText(entry.type);
        }
        if (ImGui::TableNextColumn()) {
          ImGuiStringViewText(entry.name);
        }
        ImGuiConsole::EndEntityRow(entry);
      }
    }
    ImGui::EndTable();
  }
  ImGui::InputFloat("Time Dirty", &x88_timeDirty);
  ImGuiUniqueId("Team Captain", x8c_teamCaptainId);
  ImGui::InputFloat("Time Since Melee", &x90_timeSinceMelee);
  ImGui::InputFloat("Time Since Ranged", &x94_timeSinceRanged);
})

// <- CActor
IMGUI_ENTITY_INSPECT(CPhysicsActor, CActor, PhysicsActor, {
  float mass = xe8_mass;
  if (ImGui::InputFloat("Mass", &mass)) {
    SetMass(mass);
  }
  float inertiaTensor = xf0_inertiaTensor;
  if (ImGui::InputFloat("Inertia tensor", &inertiaTensor)) {
    SetInertiaTensorScalar(inertiaTensor);
  }
  if (ImGuiVector3fInput("Velocity", x138_velocity)) {
    SetVelocityWR(x138_velocity);
  }
  if (ImGuiVector3fInput("Momentum", x150_momentum)) {
    SetMomentumWR(x150_momentum);
  }
  zeus::CVector3f force = x15c_force;
  if (ImGuiVector3fInput("Force", force)) {
    ApplyForceWR(force - x15c_force, zeus::CAxisAngle());
  }
  zeus::CVector3f impulse = x168_impulse;
  if (ImGuiVector3fInput("Impulse", x168_impulse)) {
    ApplyImpulseWR(impulse - x168_impulse, zeus::CAxisAngle());
  }
  BITFIELD_CHECKBOX("Movable", xf8_24_movable);
  BITFIELD_CHECKBOX("Angular enabled", xf8_25_angularEnabled);
  BITFIELD_CHECKBOX("Standard collider", xf9_standardCollider);
})
IMGUI_ENTITY_INSPECT(MP1::CDroneLaser, CActor, DroneLaser, {})
IMGUI_ENTITY_INSPECT(CEffect, CActor, Effect, {})
IMGUI_ENTITY_INSPECT(CFire, CActor, Fire, {})
IMGUI_ENTITY_INSPECT(CFishCloud, CActor, FishCloud, {})
IMGUI_ENTITY_INSPECT(CFishCloudModifier, CActor, FishCloudModifier, {})
IMGUI_ENTITY_INSPECT(MP1::CFlaahgraPlants, CActor, FlaahgraPlants, {})
IMGUI_ENTITY_INSPECT(MP1::CFlaahgraRenderer, CActor, FlaahgraRenderer, {})
IMGUI_ENTITY_INSPECT(MP1::COmegaPirate::CFlash, CActor, OmegaPirateFlash, {})
IMGUI_ENTITY_INSPECT(CGameCamera, CActor, GameCamera, {})
IMGUI_ENTITY_INSPECT(CGameLight, CActor, GameLight, {
  ImGuiVector3fInput("Position", xec_light.x0_pos);
  ImGuiVector3fInput("Direction", xec_light.xc_dir);
  if (ImGuiColorInput("Color", xec_light.x18_color)) {
    xec_light.SetColor(xec_light.x18_color); // set dirty flags
  }
  ImGuiEnumInput("Type", xec_light.x1c_type);
  ImGui::DragFloat("Spot cutoff", &xec_light.x20_spotCutoff, 0.1f);
  {
    std::array<float, 3> att{xec_light.x24_distC, xec_light.x28_distL, xec_light.x2c_distQ};
    if (ImGui::DragFloat3("Attenuation", att.data())) {
      xec_light.SetAttenuation(att[0], att[1], att[2]);
    }
    ImGui::SameLine();
    ImGui::TextUnformatted("(?)");
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Constant | Linear | Quadratic");
    }
  }
  {
    std::array<float, 3> angleAtt{xec_light.x30_angleC, xec_light.x34_angleL, xec_light.x38_angleQ};
    if (ImGui::DragFloat3("Angle Atten", angleAtt.data())) {
      xec_light.SetAngleAttenuation(angleAtt[0], angleAtt[1], angleAtt[2]);
    }
    ImGui::SameLine();
    ImGui::TextUnformatted("(?)");
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Constant | Linear | Quadratic");
    }
  }
  ImGui::Text("Calculated radius: %.3f", xec_light.GetRadius());
  ImGui::Text("Calculated intensity: %.3f", xec_light.GetIntensity());
})
IMGUI_ENTITY_INSPECT(MP1::CIceAttackProjectile, CActor, IceAttackProjectile, {})
IMGUI_ENTITY_INSPECT(CRepulsor, CActor, Repulsor, {})
IMGUI_ENTITY_INSPECT(CScriptAiJumpPoint, CActor, ScriptAIJumpPoint, {})
IMGUI_ENTITY_INSPECT(CScriptBeam, CActor, ScriptBeam, {})
IMGUI_ENTITY_INSPECT(CScriptCameraHint, CActor, ScriptCameraHint, {})
IMGUI_ENTITY_INSPECT(CScriptCameraHintTrigger, CActor, ScriptCameraHintTrigger, {})
IMGUI_ENTITY_INSPECT(CScriptCameraPitchVolume, CActor, ScriptCameraPitchVolume, {})
IMGUI_ENTITY_INSPECT(CScriptCameraWaypoint, CActor, ScriptCameraWaypoint, {})
IMGUI_ENTITY_INSPECT(CScriptCoverPoint, CActor, ScriptCoverPoint, {})
IMGUI_ENTITY_INSPECT(CScriptDamageableTrigger, CActor, ScriptDamageableTrigger, {})
IMGUI_ENTITY_INSPECT(CScriptDebugCameraWaypoint, CActor, ScriptDebugCameraWaypoint, {})
IMGUI_ENTITY_INSPECT(CScriptEffect, CActor, ScriptEffect, {
  BITFIELD_CHECKBOX("Enabled", x110_24_enable);
  BITFIELD_CHECKBOX("No Timer Unless Area Occluded", x110_25_noTimerUnlessAreaOccluded);
  BITFIELD_CHECKBOX("Rebuild Systems On Activate", x110_26_rebuildSystemsOnActivate);
  BITFIELD_CHECKBOX("Use Rate Inverse Camera Distance", x110_27_useRateInverseCamDist);
  BITFIELD_CHECKBOX("Combat Visor Visible", x110_28_combatVisorVisible);
  BITFIELD_CHECKBOX("Thermal Visor Visible", x110_29_thermalVisorVisible);
  BITFIELD_CHECKBOX("X-Ray Visor Visible", x110_30_xrayVisorVisible);
  BITFIELD_CHECKBOX("Any Visor Visible", x110_31_anyVisorVisible);
  BITFIELD_CHECKBOX("Use Rate Camera Distance Range", x111_24_useRateCamDistRange);
  BITFIELD_CHECKBOX("Die When Systems Done", x111_25_dieWhenSystemsDone);
  BITFIELD_CHECKBOX("Can Render", x111_26_canRender);
  if (ImGui::DragFloat("Rate Inverse Camera Distance", &x114_rateInverseCamDist, 0.1f)) {
    x118_rateInverseCamDistSq = x114_rateInverseCamDist * x114_rateInverseCamDist;
  }
  ImGui::DragFloat("Rate Inverse Camera Distance Rate", &x11c_rateInverseCamDistRate, 0.1f);
  ImGui::DragFloat("Rate Camera Distance Range Min", &x120_rateCamDistRangeMin, 0.1f);
  ImGui::DragFloat("Rate Camera Distance Range Max", &x124_rateCamDistRangeMax, 0.1f);
  ImGui::DragFloat("Rate Camera Distance Range Far Rate", &x128_rateCamDistRangeFarRate, 0.1f);
  ImGui::DragFloat("Remaining Time", &x12c_remTime, 0.1f);
  ImGui::DragFloat("Duration", &x130_duration, 0.1f);
  ImGui::DragFloat("Duration Reset While Visible", &x134_durationResetWhileVisible, 0.1f);
  ImGuiUniqueId("Trigger ID", x13c_triggerId);
  ImGui::DragFloat("Destroy Delay Timer", &x140_destroyDelayTimer, 0.1f);
})
IMGUI_ENTITY_INSPECT(CScriptEMPulse, CActor, ScriptEMPulse, {})
IMGUI_ENTITY_INSPECT(CScriptGrapplePoint, CActor, ScriptGrapplePoint, {})
IMGUI_ENTITY_INSPECT(CScriptMazeNode, CActor, ScriptMazeNode, {})
IMGUI_ENTITY_INSPECT(CScriptPlayerHint, CActor, ScriptPlayerHint, {})
IMGUI_ENTITY_INSPECT(CScriptPointOfInterest, CActor, ScriptPointOfInterest, {})
IMGUI_ENTITY_INSPECT(CScriptShadowProjector, CActor, ScriptShadowProjector, {})
IMGUI_ENTITY_INSPECT(CScriptSound, CActor, ScriptSound, {})
IMGUI_ENTITY_INSPECT(CScriptSpecialFunction, CActor, ScriptSpecialFunction, {})
IMGUI_ENTITY_INSPECT(CScriptSpiderBallAttractionSurface, CActor, ScriptSpiderballAttractionSurface, {})
IMGUI_ENTITY_INSPECT(CScriptSpiderBallWaypoint, CActor, ScriptSpiderBallWaypoint, {})
IMGUI_ENTITY_INSPECT(CScriptTargetingPoint, CActor, ScriptTargetingPoint, {})
IMGUI_ENTITY_INSPECT(CScriptTrigger, CActor, ScriptTrigger, {})
IMGUI_ENTITY_INSPECT(CScriptVisorFlare, CActor, ScriptVisorFlare, {})
IMGUI_ENTITY_INSPECT(CScriptVisorGoo, CActor, ScriptVisorGoo, {})
IMGUI_ENTITY_INSPECT(CScriptWaypoint, CActor, ScriptWaypoint, {})
IMGUI_ENTITY_INSPECT(MP1::CShockWave, CActor, ShockWave, {})
IMGUI_ENTITY_INSPECT(CSnakeWeedSwarm, CActor, SnakeWeedSwarm, {})
IMGUI_ENTITY_INSPECT(CWallCrawlerSwarm, CActor, WallCrawlerSwarm, {})
IMGUI_ENTITY_INSPECT(CWeapon, CActor, Weapon, {})

// <- CEffect
IMGUI_ENTITY_INSPECT(CExplosion, CEffect, Explosion, {})
IMGUI_ENTITY_INSPECT(CHUDBillboardEffect, CEffect, HUDBillboardEffect, {})
IMGUI_ENTITY_INSPECT(CIceImpact, CEffect, IceImpact, {})

// <- CGameCamera
IMGUI_ENTITY_INSPECT(CBallCamera, CGameCamera, BallCamera, {})
IMGUI_ENTITY_INSPECT(CCinematicCamera, CGameCamera, CinematicCamera, {})
IMGUI_ENTITY_INSPECT(CFirstPersonCamera, CGameCamera, FirstPersonCamera, {})
IMGUI_ENTITY_INSPECT(CInterpolationCamera, CGameCamera, InterpolationCamera, {})
IMGUI_ENTITY_INSPECT(CPathCamera, CGameCamera, PathCamera, {})
IMGUI_ENTITY_INSPECT(CScriptSpindleCamera, CGameCamera, ScriptSpindleCamera, {})

// <- CScriptTrigger
IMGUI_ENTITY_INSPECT(MP1::CPhazonPool, CScriptTrigger, PhazonPool, {})
IMGUI_ENTITY_INSPECT(CScriptBallTrigger, CScriptTrigger, ScriptBallTrigger, {})
IMGUI_ENTITY_INSPECT(CScriptSteam, CScriptTrigger, ScriptSteam, {})
IMGUI_ENTITY_INSPECT(CScriptWater, CScriptTrigger, ScriptWater, {})

// <- CWeapon
IMGUI_ENTITY_INSPECT(CBomb, CWeapon, Bomb, {})
IMGUI_ENTITY_INSPECT(CGameProjectile, CWeapon, GameProjectile, {})
IMGUI_ENTITY_INSPECT(CPowerBomb, CWeapon, PowerBomb, {})

// <- CGameProjectile
IMGUI_ENTITY_INSPECT(CBeamProjectile, CGameProjectile, BeamProjectile, {})
IMGUI_ENTITY_INSPECT(CEnergyProjectile, CGameProjectile, EnergyProjectile, {})
IMGUI_ENTITY_INSPECT(CFlameThrower, CGameProjectile, FlameThrower, {})
IMGUI_ENTITY_INSPECT(CNewFlameThrower, CGameProjectile, NewFlameThrower, {})
IMGUI_ENTITY_INSPECT(CWaveBuster, CGameProjectile, WaveBuster, {})

// <- CBeamProjectile
IMGUI_ENTITY_INSPECT(CElectricBeamProjectile, CBeamProjectile, ElectricBeamProjectile, {})
IMGUI_ENTITY_INSPECT(CPlasmaProjectile, CBeamProjectile, PlasmaProjectile, {})

// <- CEnergyProjectile
IMGUI_ENTITY_INSPECT(MP1::CFlaahgraProjectile, CEnergyProjectile, FlaahgraProjectile, {})
IMGUI_ENTITY_INSPECT(MP1::CMetroidPrimeProjectile, CEnergyProjectile, MetroidPrimeProjectile, {})
IMGUI_ENTITY_INSPECT(CTargetableProjectile, CEnergyProjectile, TargetableProjectile, {})

// <- CPhysicsActor
IMGUI_ENTITY_INSPECT(CAi, CPhysicsActor, AI, {})
IMGUI_ENTITY_INSPECT(CAmbientAI, CPhysicsActor, AmbientAI, {})
IMGUI_ENTITY_INSPECT(MP1::CBouncyGrenade, CPhysicsActor, BouncyGrenade, {})
IMGUI_ENTITY_INSPECT(CCollisionActor, CPhysicsActor, CollisionActor, {})
IMGUI_ENTITY_INSPECT(MP1::CGrenadeLauncher, CPhysicsActor, GrenadeLauncher, {})
IMGUI_ENTITY_INSPECT(MP1::CMetroidPrimeExo::CPhysicsDummy, CPhysicsActor, MetroidPrimeExoPhysicsDummy, {})
IMGUI_ENTITY_INSPECT(CPlayer, CPhysicsActor, Player, {
  if (ImGui::CollapsingHeader("Player Gun")) {
    auto* gun = GetPlayerGun();
    ImGui::Text("Last Fire Button States: 0x%08X", gun->x2ec_lastFireButtonStates);
    ImGui::Text("Pressed Fire Button States: 0x%08X", gun->x2f0_pressedFireButtonStates);
    ImGui::Text("Fire Button States: 0x%08X", gun->x2f4_fireButtonStates);
    ImGui::Text("State Flags: 0x%08X", gun->x2f8_stateFlags);
    ImGui::Text("Fidget Anim Bits: 0x%08X", gun->x2fc_fidgetAnimBits);
    ImGui::Text("Remaining Missiles: %i", gun->x300_remainingMissiles);
    ImGui::Text("Bomb Count: %i", gun->x308_bombCount);
    ImGui::Text("Current Beam: %s", magic_enum::enum_name(gun->x310_currentBeam).data());
    ImGui::Text("Next Beam: %s", magic_enum::enum_name(gun->x314_nextBeam).data());
  }
})
IMGUI_ENTITY_INSPECT(CScriptActor, CPhysicsActor, ScriptActor, {
  if (ImGui::Button("Edit Damage Vulnerability")) {
    m_editingDamageVulnerability = true;
  }
  x268_damageVulnerability.ImGuiEditWindow("Damage Vulnerability - Script Actor", m_editingDamageVulnerability);

  bool modelFlagsChanged = false;
  ImGui::DragFloat("Fade In Time", &x2d0_fadeInTime, 0.1f);
  ImGui::DragFloat("Fade Out Time", &x2d4_fadeOutTime, 0.1f);
  ImGui::SliderFloat("X-Ray Alpha", &x2dc_xrayAlpha, 0.0f, 1.f);
  ImGui::SameLine();
  BITFIELD_CHECKBOX("Enabled", x2e2_27_xrayAlphaEnabled, { modelFlagsChanged = true; });
  BITFIELD_CHECKBOX("Disable Thermal Hot Z-test", x2e2_24_noThermalHotZ, { modelFlagsChanged = true; });
  BITFIELD_CHECKBOX("Dead", x2e2_25_dead); // onclick -> EScriptObjectMessage::Reset?
  BITFIELD_CHECKBOX("Animating", x2e2_26_animating);
  BITFIELD_CHECKBOX("Scale Advancement Delta", x2e2_30_scaleAdvancementDelta);
  BITFIELD_CHECKBOX("Material Flag 54", x2e2_31_materialFlag54);
  BITFIELD_CHECKBOX("Is Player Actor", x2e3_24_isPlayerActor);
  ImGuiUniqueId("Trigger ID", x2e0_triggerId);
  x2e2_29_processModelFlags =
      modelFlagsChanged || x2e2_27_xrayAlphaEnabled || x2e2_24_noThermalHotZ || x2d8_shaderIdx != 0;
})
IMGUI_ENTITY_INSPECT(CScriptDebris, CPhysicsActor, ScriptDebris, {})
IMGUI_ENTITY_INSPECT(CScriptDock, CPhysicsActor, ScriptDock, {
  if (x260_area != kInvalidAreaId) {
    const auto* dock = g_StateManager->GetWorld()->GetArea(x260_area)->GetDock(x25c_dock);
    if (dock != nullptr) {
      auto areaId = dock->GetConnectedAreaId(dock->GetReferenceCount());
      if (areaId != kInvalidAreaId) {
        CAssetId stringId = g_StateManager->GetWorld()->GetArea(areaId)->IGetStringTableAssetId();
        ImGuiStringViewText(fmt::format(FMT_STRING("Connected Area: {}"), ImGuiLoadStringTable(stringId, 0)));
      }
    }
  }
  ImGuiEnumInput("Dock State", x264_dockState);
  BITFIELD_CHECKBOX("Dock Referenced", x268_24_dockReferenced);
  BITFIELD_CHECKBOX("Load Connected", x268_25_loadConnected);
  BITFIELD_CHECKBOX("Area Post Constructed", x268_26_areaPostConstructed);
});
IMGUI_ENTITY_INSPECT(CScriptDoor, CPhysicsActor, ScriptDoor, {
  ImGui::DragFloat("Animation Length", &x258_animLen, 0.1f);
  ImGui::DragFloat("Animation Time", &x25c_animTime, 0.1f);
  ImGuiEnumInput("Door State", x260_doorAnimState);
  // TODO: AABox
  ImGuiUniqueId("Partner1", x27c_partner1);
  ImGuiUniqueId("Partner2", x27e_partner2);
  ImGuiUniqueId("Previous Door", x280_prevDoor);
  ImGuiUniqueId("Dock", x282_dockId);
  // TODO: model aabox
  ImGuiVector3fInput("Orbit Position", x29c_orbitPos);
  BITFIELD_CHECKBOX("Closing", x2a8_24_closing);
  BITFIELD_CHECKBOX("Was Open", x2a8_25_wasOpen);
  BITFIELD_CHECKBOX("Is Open", x2a8_26_isOpen);
  BITFIELD_CHECKBOX("Open Conditions Met", x2a8_27_conditionsMet);
  BITFIELD_CHECKBOX("Projectiles Can Collide", x2a8_28_projectilesCollide);
  BITFIELD_CHECKBOX("Is Ball Door", x2a8_29_ballDoor);
  BITFIELD_CHECKBOX("Will Close", x2a8_30_doClose);
})
IMGUI_ENTITY_INSPECT(CScriptGunTurret, CPhysicsActor, ScriptGunTurret, {})
IMGUI_ENTITY_INSPECT(CScriptPickup, CPhysicsActor, ScriptPickup, {
  ImGuiEnumInput("Item Type", x258_itemType);
  ImGui::DragInt("Amount", &x25c_amount);
  ImGui::DragInt("Capacity", &x260_capacity);
  ImGui::SliderFloat("Possibility", &x264_possibility, 0.f, 100.f);
  ImGui::DragFloat("Fade In Time", &x268_fadeInTime);
  ImGui::DragFloat("Lifetime", &x26c_lifeTime);
  ImGui::DragFloat("Current Time", &x270_curTime);
  ImGui::DragFloat("Tractor Time", &x274_tractorTime);
  ImGui::DragFloat("Delay Timer", &x278_delayTimer);
})
IMGUI_ENTITY_INSPECT(CScriptPlatform, CPhysicsActor, ScriptPlatform, {
  ImGuiUniqueId("Current Waypoint", x258_currentWaypoint);
  ImGuiUniqueId("Target Waypoint", x25a_targetWaypoint);
})

// <- CScriptActor
IMGUI_ENTITY_INSPECT(MP1::CActorContraption, CScriptActor, ActorContraption, {})
IMGUI_ENTITY_INSPECT(CScriptPlayerActor, CScriptActor, PlayerActor, {
  ImGuiAnimRes("Suit Resource", x2e8_suitRes);
  ImGuiEnumInput("Beam", x304_beam);
  ImGuiEnumInput("Suit", x308_suit);
})

// <- CScriptPlatform
IMGUI_ENTITY_INSPECT(MP1::CRipperControlledPlatform, CScriptPlatform, RipperControlledPlatform, {})

// <- CAi
IMGUI_ENTITY_INSPECT(CDestroyableRock, CAi, DestroyableRock, {})
IMGUI_ENTITY_INSPECT(CPatterned, CAi, Patterned, {
  BITFIELD_CHECKBOX("Enable state machine", x403_25_enableStateMachine);
  ImGui::Text("Body state:");
  ImGui::SameLine();
  ImGuiStringViewText(pas::AnimationStateToStr(x450_bodyController->GetCurrentStateId()));
  if (ImGui::Button("Burn")) {
    Burn(1.f, 1.f);
  }
  if (ImGui::Button("Shock")) {
    Shock(*g_StateManager, 1.f, 1.f);
  }
  if (ImGui::Button("Freeze")) {
    Freeze(*g_StateManager, GetTranslation(), zeus::skZero3f, 1.f);
  }
})

// <- CPatterned
IMGUI_ENTITY_INSPECT(MP1::CAtomicAlpha, CPatterned, AtomicAlpha, {})
IMGUI_ENTITY_INSPECT(MP1::CAtomicBeta, CPatterned, AtomicBeta, {})
IMGUI_ENTITY_INSPECT(MP1::CBabygoth, CPatterned, Babygoth, {})
IMGUI_ENTITY_INSPECT(MP1::CBeetle, CPatterned, Beetle, {})
IMGUI_ENTITY_INSPECT(MP1::CBloodFlower, CPatterned, BloodFlower, {})
IMGUI_ENTITY_INSPECT(MP1::CBurrower, CPatterned, Burrower, {})
IMGUI_ENTITY_INSPECT(MP1::CChozoGhost, CPatterned, ChozoGhost, {})
IMGUI_ENTITY_INSPECT(MP1::CDrone, CPatterned, Drone, {})
IMGUI_ENTITY_INSPECT(MP1::CElitePirate, CPatterned, ElitePirate, {})
IMGUI_ENTITY_INSPECT(MP1::CEnergyBall, CPatterned, EnergyBall, {})
IMGUI_ENTITY_INSPECT(MP1::CEyeball, CPatterned, EyeBall, {})
IMGUI_ENTITY_INSPECT(MP1::CFireFlea, CPatterned, FireFlea, {})
IMGUI_ENTITY_INSPECT(MP1::CFlaahgra, CPatterned, Flaahgra, {})
IMGUI_ENTITY_INSPECT(MP1::CFlaahgraTentacle, CPatterned, FlaahgraTentacle, {})
IMGUI_ENTITY_INSPECT(MP1::CFlickerBat, CPatterned, FlickerBat, {})
IMGUI_ENTITY_INSPECT(MP1::CFlyingPirate, CPatterned, FlyingPirate, {})
IMGUI_ENTITY_INSPECT(MP1::CIceSheegoth, CPatterned, IceSheegoth, {})
IMGUI_ENTITY_INSPECT(MP1::CJellyZap, CPatterned, JellyZap, {})
IMGUI_ENTITY_INSPECT(MP1::CMagdolite, CPatterned, Magdolite, {})
IMGUI_ENTITY_INSPECT(MP1::CMetaree, CPatterned, Metaree, {})
IMGUI_ENTITY_INSPECT(MP1::CMetroid, CPatterned, Metroid, {})
IMGUI_ENTITY_INSPECT(MP1::CMetroidBeta, CPatterned, MetroidBeta, {})
IMGUI_ENTITY_INSPECT(MP1::CMetroidPrimeEssence, CPatterned, MetroidPrimeEssence, {})
IMGUI_ENTITY_INSPECT(MP1::CMetroidPrimeExo, CPatterned, MetroidPrimeExo, {})
IMGUI_ENTITY_INSPECT(MP1::CNewIntroBoss, CPatterned, NewIntroBoss, {})
IMGUI_ENTITY_INSPECT(MP1::CPhazonHealingNodule, CPatterned, PhazonHealingNodule, {})
IMGUI_ENTITY_INSPECT(MP1::CPuddleSpore, CPatterned, PuddleSpore, {})
IMGUI_ENTITY_INSPECT(MP1::CPuddleToadGamma, CPatterned, PuddleToadGamma, {})
IMGUI_ENTITY_INSPECT(MP1::CPuffer, CPatterned, Puffer, {})
IMGUI_ENTITY_INSPECT(MP1::CRidley, CPatterned, Ridley, {})
IMGUI_ENTITY_INSPECT(MP1::CRipper, CPatterned, Ripper, {})
IMGUI_ENTITY_INSPECT(MP1::CSpacePirate, CPatterned, SpacePirate, {})
IMGUI_ENTITY_INSPECT(MP1::CSpankWeed, CPatterned, SpankWeed, {})
IMGUI_ENTITY_INSPECT(MP1::CThardus, CPatterned, Thardus, {})
IMGUI_ENTITY_INSPECT(MP1::CThardusRockProjectile, CPatterned, ThardusRockProjectile, {})
IMGUI_ENTITY_INSPECT(MP1::CTryclops, CPatterned, Tryclops, {})
IMGUI_ENTITY_INSPECT(CWallWalker, CPatterned, WallWalker, {})
IMGUI_ENTITY_INSPECT(MP1::CParasite, CWallWalker, Parasite, {})
IMGUI_ENTITY_INSPECT(MP1::CSeedling, CWallWalker, Seedling, {})
IMGUI_ENTITY_INSPECT(MP1::CWarWasp, CPatterned, WarWasp, {})
IMGUI_ENTITY_INSPECT(MP1::COmegaPirate, MP1::CElitePirate, OmegaPirate, {})
} // namespace metaforce
