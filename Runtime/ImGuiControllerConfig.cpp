#include "Runtime/ImGuiControllerConfig.hpp"

#include "Runtime/Streams/CFileOutStream.hpp"
#include "Runtime/Streams/ContainerReaders.hpp"
#include "Runtime/Streams/ContainerWriters.hpp"

#include "aurora/pad.hpp"
#include "aurora/aurora.hpp"

#include <imgui.h>

namespace metaforce {
ImGuiControllerConfig::Button::Button(CInputStream& in)
: button(in.Get<s32>())
, uvX(in.Get<u32>())
, uvY(in.Get<u32>())
, width(in.Get<u32>())
, height(in.Get<u32>())
, offX(in.Get<float>())
, offY(in.Get<float>()) {}

void ImGuiControllerConfig::Button::PutTo(COutputStream& out) const {
  out.Put(button);
  out.Put(uvX);
  out.Put(uvY);
  out.Put(width);
  out.Put(height);
  out.Put(offX);
  out.Put(offY);
}

ImGuiControllerConfig::ControllerAtlas::ControllerAtlas(CInputStream& in) : name(in.Get<std::string>()) {
  u32 vidPidCount = in.Get<u32>();
  vidPids.reserve(vidPidCount);

  for (u32 i = 0; i < vidPidCount; ++i) {
    u16 vid = static_cast<u16>(in.Get<s16>());
    u16 pid = static_cast<u16>(in.Get<s16>());
    vidPids.emplace_back(vid, pid);
  }

  atlasFile = in.Get<std::string>();
  read_vector(buttons, in);
};

void ImGuiControllerConfig::ControllerAtlas::PutTo(COutputStream& out) const {
  out.Put(name);
  out.Put(static_cast<u32>(vidPids.size()));
  for (const auto& vidPid : vidPids) {
    out.Put(vidPid.first);
    out.Put(vidPid.second);
  }

  write_vector(buttons, out);
}

void ImGuiControllerConfig::show(bool& visible) {

  /** TODO:
   * - Implement multiple controllers
   * - Implement setting controller ports (except for the GameCube adapter, which is hard coded)
   * - Implement fancy graphical UI
   */

  if (!visible) {
    return;
  }

  if (m_pendingMapping != nullptr) {
    s32 nativeButton = PADGetNativeButtonPressed(m_pendingPort);
    if (nativeButton != -1) {
      m_pendingMapping->nativeButton = nativeButton;
      m_pendingMapping = nullptr;
      m_pendingPort = -1;
      PADBlockInput(false);
    }
  }

  std::vector<std::string> controllers;
  controllers.push_back("None");
  for (u32 i = 0; i < PADCount(); ++i) {
    controllers.push_back(fmt::format(FMT_STRING("{}-{}"), PADGetNameForControllerIndex(i), i));
  }

  m_pendingValid = false;
  if (ImGui::Begin("Controller Config", &visible)) {
    if (ImGui::CollapsingHeader("Ports")) {
      for (u32 i = 0; i < 4; ++i) {
        ImGui::PushID(fmt::format(FMT_STRING("PortConf-{}"), i).c_str());
        s32 index = PADGetIndexForPort(i);
        int sel = 0;
        std::string name = "None";
        const char* tmpName = PADGetName(i);
        bool changed = false;
        if (tmpName != nullptr) {
          name = fmt::format(FMT_STRING("{}-{}"), tmpName, index);
        }
        if (ImGui::BeginCombo(fmt::format(FMT_STRING("Port {}"), i + 1).c_str(), name.c_str())) {
          for (u32 j = 0; const auto& s : controllers) {
            if (ImGui::Selectable(s.c_str(), name == s)) {
              sel = j;
              changed = true;
            }
            ++j;
          }
          ImGui::EndCombo();
        }

        if (changed) {
          if (sel > 0) {
            PADSetPortForIndex(sel - 1, i);
          } else if (sel == 0) {
            PADClearPort(i);
          }
        }
        ImGui::PopID();
      }
    }
    if (ImGui::BeginTabBar("Controllers")) {
      for (u32 i = 0; i < 4; ++i) {
        if (ImGui::BeginTabItem(fmt::format(FMT_STRING("Port {}"), i + 1).c_str())) {
          ImGui::PushID(fmt::format(FMT_STRING("Port_{}"), i + 1).c_str());
          /* If the tab is changed while pending for input, cancel the pending port */
          if (m_pendingMapping != nullptr && m_pendingPort != i) {
            m_pendingMapping = nullptr;
            m_pendingValid = false;
            m_pendingPort = -1;
            PADBlockInput(false);
          }
          u32 vid, pid;
          PADGetVidPid(i, &vid, &pid);
          if (vid == 0 && pid == 0) {
            ImGui::EndTabItem();
            ImGui::PopID();
            continue;
          }
          ImGui::Text("%s", PADGetName(i));
          u32 buttonCount = 0;
          PADButtonMapping* mapping = PADGetButtonMappings(i, &buttonCount);
          if (mapping != nullptr) {
            for (u32 m = 0; m < buttonCount; ++m) {
              const char* padName = PADGetButtonName(mapping[m].padButton);
              if (padName == nullptr) {
                continue;
              }
              ImGui::PushID(padName);
              bool pressed = ImGui::Button(padName);
              ImGui::SameLine();
              ImGui::Text("%s", PADGetNativeButtonName(mapping[m].nativeButton));

              if (pressed && m_pendingMapping == nullptr) {
                m_pendingMapping = &mapping[m];
                m_pendingPort = i;
                PADBlockInput(true);
              }

              if (m_pendingMapping == &mapping[m]) {
                m_pendingValid = true;
                ImGui::SameLine();
                ImGui::Text(" - Waiting for button...");
              }
              ImGui::PopID();
            }
          }

          if (ImGui::CollapsingHeader("Dead-zones")) {
            PADDeadZones* deadZones = PADGetDeadZones(i);
            ImGui::Checkbox("Use Dead-zones", &deadZones->useDeadzones);
            float tmp = static_cast<float>(deadZones->stickDeadZone * 100.f) / 32767.f;
            if (ImGui::DragFloat("Left Stick", &tmp, 0.5f, 0.f, 100.f, "%.3f%%")) {
              deadZones->stickDeadZone = static_cast<u16>((tmp / 100.f) * 32767);
            }
            tmp = static_cast<float>(deadZones->substickDeadZone * 100.f) / 32767.f;
            if (ImGui::DragFloat("Right Stick", &tmp, 0.5f, 0.f, 100.f, "%.3f%%")) {
              deadZones->substickDeadZone = static_cast<u16>((tmp / 100.f) * 32767);
            }
            ImGui::Checkbox("Emulate Triggers", &deadZones->emulateTriggers);
            tmp = static_cast<float>(deadZones->leftTriggerActivationZone * 100.f) / 32767.f;
            if (ImGui::DragFloat("Left Trigger Activation", &tmp, 0.5f, 0.f, 100.f, "%.3f%%")) {
              deadZones->leftTriggerActivationZone = static_cast<u16>((tmp / 100.f) * 32767);
            }
            tmp = static_cast<float>(deadZones->rightTriggerActivationZone * 100.f) / 32767.f;
            if (ImGui::DragFloat("Right Trigger Activation", &tmp, 0.5f, 0.f, 100.f, "%.3f%%")) {
              deadZones->rightTriggerActivationZone = static_cast<u16>((tmp / 100.f) * 32767);
            }
          }
          ImGui::PopID();
          ImGui::EndTabItem();
        }
      }
      ImGui::EndTabBar();
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
    /* TODO: Atlas editor */
    ImGui::Separator();
    if (ImGui::Button("Save Controller Database")) {
      CFileOutStream out("ControllerAtlases.ctrdb");
      out.WriteUint32(SLITTLE('CTDB'));
      out.WriteUint32(1); // Version
      write_vector(m_controllerAtlases, out);
    }
    ImGui::SameLine();
    if (ImGui::Button("Export") && m_currentAtlas != nullptr) {
      CFileOutStream out("test.ctratlas");
      out.Put(SLITTLE('CTRA'));
      out.Put(1); // Version
      out.Put(*m_currentAtlas);
    }

    /* TODO: Import logic */
  }
  ImGui::End();
}
} // namespace metaforce
