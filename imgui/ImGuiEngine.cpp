#include "ImGuiEngine.hpp"

#include "aurora_imgui.h"

#include "athena/Compression.hpp"

#define STBI_NO_STDIO
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

extern "C" const uint8_t NOTO_MONO_FONT[];
extern "C" const size_t NOTO_MONO_FONT_SZ;
extern "C" const size_t NOTO_MONO_FONT_DECOMPRESSED_SZ;
extern "C" const uint8_t METAFORCE_ICON[];
extern "C" const size_t METAFORCE_ICON_SZ;

namespace metaforce {
ImFont* ImGuiEngine::fontNormal;
ImFont* ImGuiEngine::fontLarge;
ImTextureID ImGuiEngine::metaforceIcon;

void ImGuiEngine_Initialize(float scale) {
  ImGui::GetCurrentContext();
  ImGuiIO& io = ImGui::GetIO();

  auto* fontData = new uint8_t[NOTO_MONO_FONT_DECOMPRESSED_SZ];
  athena::io::Compression::decompressZlib(static_cast<const atUint8*>(NOTO_MONO_FONT), atUint32(NOTO_MONO_FONT_SZ),
                                          fontData, NOTO_MONO_FONT_DECOMPRESSED_SZ);

  ImFontConfig fontConfig{};
  fontConfig.FontData = fontData;
  fontConfig.FontDataSize = int(NOTO_MONO_FONT_DECOMPRESSED_SZ);
  fontConfig.SizePixels = std::floor(14.f * scale);
  snprintf(static_cast<char*>(fontConfig.Name), sizeof(fontConfig.Name), "Noto Mono Regular, %dpx",
           static_cast<int>(fontConfig.SizePixels));
  ImGuiEngine::fontNormal = io.Fonts->AddFont(&fontConfig);
  fontConfig.FontDataOwnedByAtlas = false; // first one took ownership
  fontConfig.SizePixels = std::floor(24.f * scale);
  snprintf(static_cast<char*>(fontConfig.Name), sizeof(fontConfig.Name), "Noto Mono Regular, %dpx",
           static_cast<int>(fontConfig.SizePixels));
  ImGuiEngine::fontLarge = io.Fonts->AddFont(&fontConfig);

  ImGui::GetStyle().ScaleAllSizes(scale);
}

void ImGuiEngine_AddTextures(ImGuiState& state, const DeviceHolder& gpu) {
  int iconWidth = 0;
  int iconHeight = 0;
  auto* iconData = stbi_load_from_memory(static_cast<const stbi_uc*>(METAFORCE_ICON), int(METAFORCE_ICON_SZ),
                                         &iconWidth, &iconHeight, nullptr, 4);
  ImGuiEngine::metaforceIcon = ImGuiEngine_AddTexture(state, gpu, iconWidth, iconHeight,
                                                      {iconData, static_cast<size_t>(iconWidth * iconHeight * 4)});
  stbi_image_free(iconData);
}
} // namespace metaforce
