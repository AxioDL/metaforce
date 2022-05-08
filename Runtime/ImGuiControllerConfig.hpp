#pragma once

#include "Runtime/GCNTypes.hpp"
#include "aurora/pad.hpp"

#include <array>
#include <chrono>
#include <string>
#include <vector>

namespace metaforce {
class ImGuiControllerConfig {
  struct Button {
    u32 button; // the SDL button this entry corresponds to
    u32 uvX;    // Offset if icon image in atlas from left (in pixels)
    u32 uvY;    // Offset if icon image in atlas from top (in pixels)
    u32 width;  // Width of button image (in pixels)
    u32 height; // Height of button image (in pixels)
    float offX; // Offset from left of config window
    float offY; // Offset from top of config window
  };

  struct ControllerMapping {
    std::string name;
    std::pair<u32, u32> vidPid;
    std::string atlasFile; // Path to atlas relative to controller definition
    std::vector<Button> buttons;
  };

public:
  void show(bool& visible);

private:
  void showEditor(bool& visible);

  PADButtonMapping* m_pendingMapping = nullptr;
  bool m_editorVisible = false;

  std::array<ControllerMapping*, 4> m_controllers;
  std::vector<ControllerMapping> m_mappings;
};
} // namespace metaforce