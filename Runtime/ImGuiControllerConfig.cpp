#include "Runtime/ImGuiControllerConfig.hpp"

#include "aurora/pad.hpp"

#include <imgui.h>

namespace metaforce {

void ImGuiControllerConfig::show(bool& visible) {
  /** TODO:
   * - Implement multiple controllers
   * - Implement setting controller ports
   * - Implement fancy graphical UI
   */

  if (!visible) {
    return;
  }

  if (m_pendingMapping != nullptr) {
    s32 nativeButton = PADGetNativeButtonPressed(0);
    if (nativeButton != -1) {
      m_pendingMapping->nativeButton = nativeButton;
      m_pendingMapping = nullptr;
      PADBlockInput(false);
    }
  }

  if (ImGui::Begin("Controller Config", &visible)) {
    ImGui::Text("%s", PADGetName(0));
    u32 buttonCount = 0;
    PADButtonMapping* mapping = PADGetButtonMappings(0, &buttonCount);
    if (mapping != nullptr) {
      for (u32 i = 0; i < buttonCount; ++i) {
        bool pressed = ImGui::Button(PADGetButtonName(mapping[i].padButton));
        ImGui::SameLine();
        ImGui::Text("%s", PADGetNativeButtonName(mapping[i].nativeButton));

        if (pressed && m_pendingMapping == nullptr) {
          m_pendingMapping = &mapping[i];
          PADBlockInput(true);
        }

        if (m_pendingMapping == &mapping[i]) {
          ImGui::SameLine();
          ImGui::Text(" - Waiting for button...");
        }
      }
    }

    if (ImGui::CollapsingHeader("Dead-zones")) {
      PADDeadZones* deadZones = PADGetDeadZones(0);
      ImGui::Checkbox("Use Dead-zones", &deadZones->useDeadzones);
      s32 tmp = deadZones->stickDeadZone;
      if (ImGui::DragInt("Left Stick", &tmp)) {
        deadZones->stickDeadZone = tmp;
      }
      tmp = deadZones->substickDeadZone;
      if (ImGui::DragInt("Right Stick", &tmp)) {
        deadZones->substickDeadZone = tmp;
      }
      ImGui::Checkbox("Emulate Triggers", &deadZones->emulateTriggers);
      tmp = deadZones->leftTriggerActivationZone;
      if (ImGui::DragInt("Left Trigger Activation", &tmp)) {
        deadZones->leftTriggerActivationZone = tmp;
      }
      tmp = deadZones->rightTriggerActivationZone;
      if (ImGui::DragInt("Right Trigger Activation", &tmp)) {
        deadZones->rightTriggerActivationZone = tmp;
      }
    }

    ImGui::Separator();
    if (ImGui::Button("Display Editor")) {
      m_editorVisible = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Save Mappings")) {
      PADSerializeMappings();
    }
    ImGui::SameLine();
    if (ImGui::Button("Restore Defaults")) {
      for (u32 i = 0; i < 4; ++i) {
        PADRestoreDefaultMapping(i);
      }
    }
  }
  ImGui::End();

  showEditor(m_editorVisible);
}

void ImGuiControllerConfig::showEditor(bool& visible) {
  if (!visible) {
    return;
  }

  if (ImGui::Begin("Controller Atlas Editor", &visible)) {
  }
  ImGui::End();
}
} // namespace metaforce
