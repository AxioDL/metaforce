#include "Runtime/World/CEntity.hpp"
#include "Runtime/World/CActor.hpp"
#include "Runtime/World/CAi.hpp"
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "imgui.h"

#define IMGUI_ENTITY_INSPECT(CLS, PARENT, NAME, BLOCK)                                                                 \
  std::string_view CLS::ImGuiType() { return #NAME; }                                                                  \
  void CLS::ImGuiInspect() {                                                                                           \
    PARENT::ImGuiInspect();                                                                                            \
    if (ImGui::CollapsingHeader(#NAME))                                                                                \
      BLOCK                                                                                                            \
  }

namespace metaforce {
std::string_view CEntity::ImGuiType() { return "Entity"; }

void CEntity::ImGuiInspect() {
  if (ImGui::CollapsingHeader("Entity", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("ID: %x", x8_uid.Value());
    ImGui::Text("Name: %s", x10_name.c_str());
  }
}

IMGUI_ENTITY_INSPECT(CActor, CEntity, Actor, {
  const zeus::CVector3f& pos = GetTranslation();
  ImGui::Text("Position: %f, %f, %f", pos.x(), pos.y(), pos.z());
})
IMGUI_ENTITY_INSPECT(CPhysicsActor, CActor, Physics Actor, {})
IMGUI_ENTITY_INSPECT(CAi, CPhysicsActor, AI, {})
IMGUI_ENTITY_INSPECT(CPatterned, CAi, Patterned, {})

IMGUI_ENTITY_INSPECT(CPlayer, CPhysicsActor, Player, {})
} // namespace metaforce
