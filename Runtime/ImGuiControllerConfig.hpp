#pragma once

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Streams/CInputStream.hpp"
#include "Runtime/Streams/COutputStream.hpp"
#include "aurora/pad.hpp"

#include <array>
#include <chrono>
#include <string>
#include <vector>

namespace metaforce {
class ImGuiControllerConfig {
  struct Button {
    s32 button = -1; // the SDL button this entry corresponds to
    u32 uvX = 0;    // Offset if icon image in atlas from left (in pixels)
    u32 uvY = 0;    // Offset if icon image in atlas from top (in pixels)
    u32 width = 32;  // Width of button image (in pixels)
    u32 height = 32; // Height of button image (in pixels)
    float offX = 0.f; // Offset from left of config window
    float offY = 0.f; // Offset from top of config window

    Button() = default;
    explicit Button(CInputStream& in);
    void PutTo(COutputStream& in) const;
  };

  struct ControllerAtlas {
    std::string name;
    std::vector<std::pair<u16, u16>> vidPids;
    std::string atlasFile; // Path to atlas relative to controller definition
    std::vector<Button> buttons;

    ControllerAtlas() = default;
    explicit ControllerAtlas(CInputStream& in);
    void PutTo(COutputStream& out) const;
  };

public:
  void show(bool& visible);

private:
  void showEditor(bool& visible);

  PADButtonMapping* m_pendingMapping = nullptr;
  s32 m_pendingPort = 0;
  bool m_pendingValid = false;
  bool m_editorVisible = false;


  ControllerAtlas* m_currentAtlas = nullptr;
  std::vector<ControllerAtlas> m_controllerAtlases;
};
} // namespace metaforce