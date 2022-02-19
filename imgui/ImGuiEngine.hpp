#pragma once

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <memory>

namespace metaforce {
class ImGuiEngine {
public:
  static ImFont* fontNormal;
  static ImFont* fontLarge;
  static ImTextureID metaforceIcon;
};

// Called from Rust
void ImGuiEngine_Initialize(float scale);
void ImGuiEngine_AddTextures();

struct Icon {
  std::unique_ptr<uint8_t[]> data;
  size_t size;
  uint32_t width;
  uint32_t height;
};
Icon GetIcon();
} // namespace metaforce
