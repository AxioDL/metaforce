#pragma once

#include <cstddef>
#include <memory>
#include <string>

#include "specter/Button.hpp"
#include "specter/View.hpp"

namespace specter {
class Button;
class Control;
class ViewResources;

struct IViewManager;

class ScrollView : public View {
public:
  enum class Style { Plain, ThinIndicator, SideButtons };

private:
  Style m_style;
  ScissorViewChild<View*> m_contentView;
  int m_scroll[2] = {};
  int m_targetScroll[2] = {};

  size_t m_consecutiveIdx = 0;
  double m_consecutiveScroll[16][2] = {};

  bool m_drawInd = false;
  bool m_drawSideButtons = false;

  SolidShaderVert m_verts[4];
  VertexBufferBindingSolid m_vertsBinding;

  enum class SideButtonState { None, ScrollLeft, ScrollRight } m_sideButtonState = SideButtonState::None;
  struct SideButtonBinding : IButtonBinding {
    ScrollView& m_sv;
    std::string m_leftName, m_rightName;

    SideButtonBinding(ScrollView& sv, IViewManager& vm);
    std::string_view name(const Control* control) const override;
    void down(const Button* button, const boo::SWindowCoord& coord) override;
    void up(const Button* button, const boo::SWindowCoord& coord) override;
  } m_sideButtonBind;
  ViewChild<std::unique_ptr<Button>> m_sideButtons[2];

  bool _scroll(const boo::SScrollDelta& scroll);
  int scrollAreaWidth() const;

public:
  ScrollView(ViewResources& res, View& parentView, Style style);
  void setContentView(View* v) {
    m_contentView.m_view = v;
    updateSize();
  }

  void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
  void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
  void mouseMove(const boo::SWindowCoord&) override;
  void mouseEnter(const boo::SWindowCoord&) override;
  void mouseLeave(const boo::SWindowCoord&) override;
  void scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll) override;
  int getScrollX() const { return m_scroll[0]; }
  int getScrollY() const { return m_scroll[1]; }

  int nominalWidth() const override { return subRect().size[0]; }
  int nominalHeight() const override { return subRect().size[1]; }

  void setMultiplyColor(const zeus::CColor& color) override;

  void think() override;
  void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub) override;
  void draw(boo::IGraphicsCommandQueue* gfxQ) override;
};

} // namespace specter
