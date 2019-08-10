#pragma once

#include "specter/TextView.hpp"
#include "specter/Control.hpp"
#include "specter/Icon.hpp"

namespace specter {

class Button : public Control {
public:
  enum class Style {
    Block,
    Text,
  };

private:
  Style m_style;
  IButtonBinding::MenuStyle m_menuStyle = IButtonBinding::MenuStyle::None;
  zeus::CColor m_textColor;
  zeus::CColor m_bgColor;
  std::string m_textStr;
  std::unique_ptr<TextView> m_text;
  std::unique_ptr<IconView> m_icon;

  SolidShaderVert m_verts[40];
  VertexBufferBindingSolid m_vertsBinding;

  void _loadVerts() { m_vertsBinding.load<decltype(m_verts)>(m_verts); }

  RectangleConstraint m_constraint;
  int m_nomWidth, m_nomHeight;
  int m_textWidth, m_textIconWidth;

  struct ButtonTarget : View {
    Button& m_button;

    bool m_pressed = false;
    bool m_hovered = false;

    void setInactive();
    void setHover();
    void setPressed();
    void setDisabled();

    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
    void mouseEnter(const boo::SWindowCoord&) override;
    void mouseLeave(const boo::SWindowCoord&) override;
    ButtonTarget(ViewResources& res, Button& button) : View(res, button), m_button(button) {}
  };
  ViewChild<std::unique_ptr<ButtonTarget>> m_buttonTarget;

  struct MenuTarget : View {
    Button& m_button;

    bool m_pressed = false;
    bool m_hovered = false;

    void setInactive();
    void setHover();
    void setPressed();
    void setDisabled();

    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
    void mouseEnter(const boo::SWindowCoord&) override;
    void mouseLeave(const boo::SWindowCoord&) override;
    MenuTarget(ViewResources& res, Button& button) : View(res, button), m_button(button) {}
  };
  ViewChild<std::unique_ptr<MenuTarget>> m_menuTarget;

  ViewChild<std::unique_ptr<View>> m_modalMenu;

public:
  class Resources {
    friend class ViewResources;
    friend class Button;

    void init(boo::IGraphicsDataFactory::Context& ctx, const IThemeData& theme);
    void destroy() {}
  };

  ~Button() override { closeMenu({}); }
  Button(ViewResources& res, View& parentView, IButtonBinding* controlBinding, std::string_view text,
         Icon* icon = nullptr, Style style = Style::Block, const zeus::CColor& bgColor = zeus::skWhite,
         RectangleConstraint constraint = RectangleConstraint());
  Button(ViewResources& res, View& parentView, IButtonBinding* controlBinding, std::string_view text,
         const zeus::CColor& textColor, Icon* icon = nullptr, Style style = Style::Block,
         const zeus::CColor& bgColor = zeus::skWhite, RectangleConstraint constraint = RectangleConstraint());
  void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
  void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
  void mouseMove(const boo::SWindowCoord&) override;
  void mouseLeave(const boo::SWindowCoord&) override;
  void think() override;
  void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub) override;
  void draw(boo::IGraphicsCommandQueue* gfxQ) override;

  void setText(std::string_view text, const zeus::CColor& textColor);
  void setText(std::string_view text);
  void setIcon(Icon* icon = nullptr);
  std::string_view getText() const { return m_textStr; }
  void colorGlyphs(const zeus::CColor& newColor);
  int nominalWidth() const override { return m_nomWidth; }
  int nominalHeight() const override { return m_nomHeight; }

  void closeMenu(const boo::SWindowCoord& coord);
  ViewChild<std::unique_ptr<View>>& getMenu() { return m_modalMenu; }

  void setMultiplyColor(const zeus::CColor& color) override {
    View::setMultiplyColor(color);
    m_viewVertBlock.m_color = color;
    if (m_viewVertBlockBuf)
      m_viewVertBlockBuf.access().finalAssign(m_viewVertBlock);
    m_text->setMultiplyColor(color);
    if (m_icon)
      m_icon->setMultiplyColor(color);
  }
};

} // namespace specter
