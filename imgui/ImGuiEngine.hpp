#pragma once

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include <boo/IWindow.hpp>
#include <boo/graphicsdev/IGraphicsDataFactory.hpp>

namespace metaforce {
class ImGuiEngine {
public:
  static struct Input {
    boo::SWindowCoord m_mousePos{};
    std::array<bool, 5> m_mouseButtons{};
    boo::SScrollDelta m_scrollDelta{};
    boo::EModifierKey m_modifiers{};
    std::array<bool, 512> m_keys{};
    std::vector<unsigned long> m_charCodes{};
    bool m_mouseIn = true;
  } Input;
  static ImFont* fontNormal;
  static ImFont* fontLarge;

  static void Initialize(boo::IGraphicsDataFactory* factory, boo::IWindow* window, float scale,
                         std::string_view configDir);
  static void Shutdown();

  static void Begin(float dt, float scale);
  static void End();
  static void Draw(boo::IGraphicsCommandQueue* gfxQ);

  static boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                       boo::ObjToken<boo::ITexture> texture);

private:
  static void BuildShaderDataBindings(boo::IGraphicsDataFactory::Context& ctx);
};

struct ImGuiWindowCallback : boo::IWindowCallback {
  static bool m_mouseCaptured;
  static bool m_keyboardCaptured;

  void mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods) override;
  void mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods) override;
  void mouseMove(const boo::SWindowCoord& coord) override;
  void mouseEnter(const boo::SWindowCoord& coord) override;
  void mouseLeave(const boo::SWindowCoord& coord) override;
  void scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll) override;
  void charKeyDown(unsigned long charCode, boo::EModifierKey mods, bool isRepeat) override;
  void charKeyUp(unsigned long charCode, boo::EModifierKey mods) override;
  void specialKeyDown(boo::ESpecialKey key, boo::EModifierKey mods, bool isRepeat) override;
  void specialKeyUp(boo::ESpecialKey key, boo::EModifierKey mods) override;
  void resized(const boo::SWindowRect& rect, bool sync) override;
};
} // namespace metaforce
