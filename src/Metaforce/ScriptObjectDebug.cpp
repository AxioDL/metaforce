#include "MetroidPrime/CActor.hpp"
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