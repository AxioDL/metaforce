#include "ImGuiEngine.hpp"

#include <aurora/imgui.hpp>

#include "athena/Compression.hpp"

#define STBI_NO_STDIO
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

#ifdef IMGUI_ENABLE_FREETYPE
#include "misc/freetype/imgui_freetype.h"
#endif

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
  fontConfig.SizePixels = std::floor(15.f * scale);
  snprintf(static_cast<char*>(fontConfig.Name), sizeof(fontConfig.Name), "Noto Mono Regular, %dpx",
           static_cast<int>(fontConfig.SizePixels));
  ImGuiEngine::fontNormal = io.Fonts->AddFont(&fontConfig);

  fontConfig.FontDataOwnedByAtlas = false; // first one took ownership
  fontConfig.SizePixels = std::floor(26.f * scale);
#ifdef IMGUI_ENABLE_FREETYPE
  fontConfig.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Bold;
  snprintf(static_cast<char*>(fontConfig.Name), sizeof(fontConfig.Name), "Noto Mono Bold, %dpx",
           static_cast<int>(fontConfig.SizePixels));
#else
  snprintf(static_cast<char*>(fontConfig.Name), sizeof(fontConfig.Name), "Noto Mono Regular, %dpx",
           static_cast<int>(fontConfig.SizePixels));
#endif
  ImGuiEngine::fontLarge = io.Fonts->AddFont(&fontConfig);

  ImGui::GetStyle().ScaleAllSizes(scale);
}

Icon GetIcon() {
  int iconWidth = 0;
  int iconHeight = 0;
  auto* data = stbi_load_from_memory(static_cast<const stbi_uc*>(METAFORCE_ICON), int(METAFORCE_ICON_SZ), &iconWidth,
                                     &iconHeight, nullptr, 4);
  return Icon{
      std::unique_ptr<uint8_t[]>{data},
      static_cast<size_t>(iconWidth) * static_cast<size_t>(iconHeight) * 4,
      static_cast<uint32_t>(iconWidth),
      static_cast<uint32_t>(iconHeight),
  };
}

void ImGuiEngine_AddTextures() {
  auto icon = GetIcon();
  ImGuiEngine::metaforceIcon =
      aurora::imgui::add_texture(icon.width, icon.height, {icon.data.get(), icon.size});
}
} // namespace metaforce
