#include "ImGuiEngine.hpp"

#include <aurora/imgui.hpp>

#include "Runtime/Streams/CMemoryInStream.hpp"
#include "Runtime/Streams/CZipInputStream.hpp"

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
  io.Fonts->Clear();

  auto* fontData = ImGui::MemAlloc(NOTO_MONO_FONT_DECOMPRESSED_SZ);
  {
    auto stream = std::make_unique<metaforce::CMemoryInStream>(
        static_cast<const u8*>(NOTO_MONO_FONT), NOTO_MONO_FONT_SZ, metaforce::CMemoryInStream::EOwnerShip::NotOwned);
    metaforce::CZipInputStream zipInputStream{std::move(stream)};
    zipInputStream.Get(static_cast<uint8_t*>(fontData), NOTO_MONO_FONT_DECOMPRESSED_SZ);
  }

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

  auto& style = ImGui::GetStyle();
  style = {}; // Reset sizes
  style.WindowPadding = ImVec2(15, 15);
  style.WindowRounding = 5.0f;
  style.FrameBorderSize = 1.f;
  style.FramePadding = ImVec2(5, 5);
  style.FrameRounding = 4.0f;
  style.ItemSpacing = ImVec2(12, 8);
  style.ItemInnerSpacing = ImVec2(8, 6);
  style.IndentSpacing = 25.0f;
  style.ScrollbarSize = 15.0f;
  style.ScrollbarRounding = 9.0f;
  style.GrabMinSize = 5.0f;
  style.GrabRounding = 3.0f;
  style.PopupBorderSize = 1.f;
  style.PopupRounding = 7.0;
  style.TabBorderSize = 1.f;
  style.TabRounding = 3.f;

  auto* colors = style.Colors;
  colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
  colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
  colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
  colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
  colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
  colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
  colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
  colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
  colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
  colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
  colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
  colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
  colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
  colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

  style.ScaleAllSizes(scale);
}

Icon GetIcon() {
  int iconWidth = 0;
  int iconHeight = 0;
  auto* data = stbi_load_from_memory(static_cast<const stbi_uc*>(METAFORCE_ICON), int(METAFORCE_ICON_SZ), &iconWidth,
                                     &iconHeight, nullptr, 4);
  size_t size = static_cast<size_t>(iconWidth) * static_cast<size_t>(iconHeight) * 4;
  auto ptr = std::make_unique<u8[]>(size);
  memcpy(ptr.get(), data, size);
  stbi_image_free(data);
  return Icon{
      std::move(ptr),
      size,
      static_cast<uint32_t>(iconWidth),
      static_cast<uint32_t>(iconHeight),
  };
}

void ImGuiEngine_AddTextures() {
  auto icon = GetIcon();
  ImGuiEngine::metaforceIcon = aurora::imgui::add_texture(icon.width, icon.height, {icon.data.get(), icon.size});
}
} // namespace metaforce
