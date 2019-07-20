#include "logvisor/logvisor.hpp"
#include "specter/Button.hpp"
#include "specter/ViewResources.hpp"
#include "specter/RootView.hpp"

namespace specter {
static logvisor::Module Log("specter::Button");

void Button::Resources::init(boo::IGraphicsDataFactory::Context& ctx, const IThemeData& theme) {}

Button::Button(ViewResources& res, View& parentView, IButtonBinding* controlBinding, std::string_view text, Icon* icon,
               Style style, const zeus::CColor& bgColor, RectangleConstraint constraint)
: Button(res, parentView, controlBinding, text, res.themeData().uiText(), icon, style, bgColor, constraint) {}

Button::Button(ViewResources& res, View& parentView, IButtonBinding* controlBinding, std::string_view text,
               const zeus::CColor& textColor, Icon* icon, Style style, const zeus::CColor& bgColor,
               RectangleConstraint constraint)
: Control(res, parentView, controlBinding)
, m_style(style)
, m_textColor(textColor)
, m_bgColor(bgColor)
, m_textStr(text)
, m_constraint(constraint) {
  commitResources(res, [&](boo::IGraphicsDataFactory::Context& ctx) -> bool {
    buildResources(ctx, res);
    m_vertsBinding.init(ctx, res, 40, m_viewVertBlockBuf);
    return true;
  });

  m_buttonTarget.m_view.reset(new ButtonTarget(res, *this));
  m_menuTarget.m_view.reset(new MenuTarget(res, *this));

  if (style == Style::Block) {
    zeus::CColor c1 = res.themeData().button1Inactive() * bgColor;
    zeus::CColor c2 = res.themeData().button2Inactive() * bgColor;
    m_verts[0].m_color = c1;
    m_verts[1].m_color = c2;
    m_verts[2].m_color = c1;
    m_verts[3].m_color = c2;
    m_verts[4].m_color = c2;
    for (int i = 5; i < 28; ++i)
      m_verts[i].m_color = c2;
    m_verts[31].m_color = c1;
    m_verts[32].m_color = c2;
    m_verts[33].m_color = c1;
    m_verts[34].m_color = c2;
    for (int i = 35; i < 39; ++i)
      m_verts[i].m_color = c2;
  } else {
    for (int i = 0; i < 4; ++i)
      m_verts[i].m_color = zeus::skClear;
    for (int i = 31; i < 35; ++i)
      m_verts[i].m_color = zeus::skClear;
  }
  for (int i = 28; i < 31; ++i)
    m_verts[i].m_color = m_textColor;
  _loadVerts();

  if (controlBinding)
    m_menuStyle = controlBinding->menuStyle(this);

  if (icon)
    m_icon.reset(new IconView(res, *this, *icon));

  m_text.reset(new TextView(res, *this, res.m_mainFont, TextView::Alignment::Center));
  setText(m_textStr);
}

void Button::setText(std::string_view text) { setText(text, m_textColor); }

void Button::setText(std::string_view text, const zeus::CColor& textColor) {
  m_textStr = text;
  m_textColor = textColor;

  m_text->typesetGlyphs(text, textColor);
  float pf = rootView().viewRes().pixelFactor();
  int width, height;

  if (m_style == Style::Block) {
    m_textWidth = m_text->nominalWidth();
    int nomWidth = m_textWidth + 12 * pf;
    if (m_icon)
      nomWidth += 18 * pf;
    std::pair<int, int> constraint = m_constraint.solve(nomWidth, 20 * pf);
    width = constraint.first;
    height = constraint.second;
    m_verts[0].m_pos.assign(1, height + 1, 0);
    m_verts[1].m_pos.assign(1, 1, 0);
    m_verts[2].m_pos.assign(width + 1, height + 1, 0);
    m_verts[3].m_pos.assign(width + 1, 1, 0);
    m_verts[4].m_pos.assign(width + 1, 1, 0);

    m_textIconWidth = width;
    if (m_menuStyle == IButtonBinding::MenuStyle::Primary)
      width += 12 * pf;
    else if (m_menuStyle == IButtonBinding::MenuStyle::Auxiliary)
      width += 16 * pf;

    m_verts[5].m_pos.assign(1, height + 1, 0);
    m_verts[6].m_pos.assign(1, height + 1, 0);
    m_verts[7].m_pos.assign(0, height + 1, 0);
    m_verts[8].m_pos.assign(1, 1, 0);
    m_verts[9].m_pos.assign(0, 1, 0);
    m_verts[10].m_pos.assign(0, 1, 0);

    m_verts[11].m_pos.assign(width + 2, height + 1, 0);
    m_verts[12].m_pos.assign(width + 2, height + 1, 0);
    m_verts[13].m_pos.assign(width + 1, height + 1, 0);
    m_verts[14].m_pos.assign(width + 2, 1, 0);
    m_verts[15].m_pos.assign(width + 1, 1, 0);
    m_verts[16].m_pos.assign(width + 1, 1, 0);

    m_verts[17].m_pos.assign(1, height + 2, 0);
    m_verts[18].m_pos.assign(1, height + 2, 0);
    m_verts[19].m_pos.assign(1, height + 1, 0);
    m_verts[20].m_pos.assign(width + 1, height + 2, 0);
    m_verts[21].m_pos.assign(width + 1, height + 1, 0);
    m_verts[22].m_pos.assign(width + 1, height + 1, 0);

    m_verts[23].m_pos.assign(1, 1, 0);
    m_verts[24].m_pos.assign(1, 1, 0);
    m_verts[25].m_pos.assign(1, 0, 0);
    m_verts[26].m_pos.assign(width + 1, 1, 0);
    m_verts[27].m_pos.assign(width + 1, 0, 0);

    int arrowX = m_textIconWidth + 5 * pf;
    int arrowY = 7 * pf;
    int menuBgX = m_textIconWidth;
    if (m_menuStyle == IButtonBinding::MenuStyle::Primary) {
      menuBgX = 0;
      arrowX -= 5 * pf;
    }

    m_verts[28].m_pos.assign(arrowX + 4 * pf, arrowY + 1 * pf, 0);
    m_verts[29].m_pos.assign(arrowX, arrowY + 5 * pf, 0);
    m_verts[30].m_pos.assign(arrowX + 8 * pf, arrowY + 5 * pf, 0);

    m_verts[31].m_pos.assign(menuBgX + 1, height + 1, 0);
    m_verts[32].m_pos.assign(menuBgX + 1, 1, 0);
    m_verts[33].m_pos.assign(width + 1, height + 1, 0);
    m_verts[34].m_pos.assign(width + 1, 1, 0);

    m_verts[35].m_pos.assign(m_textIconWidth, height + 1, 0);
    m_verts[36].m_pos.assign(m_textIconWidth, 1, 0);
    m_verts[37].m_pos.assign(m_textIconWidth + 1, height + 1, 0);
    m_verts[38].m_pos.assign(m_textIconWidth + 1, 1, 0);

    _loadVerts();
  } else {
    width = m_text->nominalWidth();
    height = 10 * pf;
    m_verts[0].m_pos.assign(1 * pf, -1 * pf, 0);
    m_verts[1].m_pos.assign(1 * pf, -2 * pf, 0);
    m_verts[2].m_pos.assign(width, -1 * pf, 0);
    m_verts[3].m_pos.assign(width, -2 * pf, 0);

    int arrowX = width + 5 * pf;
    m_verts[28].m_pos.assign(arrowX + 4 * pf, 1 * pf, 0);
    m_verts[29].m_pos.assign(arrowX, 5 * pf, 0);
    m_verts[30].m_pos.assign(arrowX + 8 * pf, 5 * pf, 0);

    m_textWidth = width;
    m_textIconWidth = width;

    int arrowLineWidth = 7 * pf;
    if (m_menuStyle != IButtonBinding::MenuStyle::None) {
      width += 13 * pf;
      if (m_menuStyle == IButtonBinding::MenuStyle::Primary) {
        arrowLineWidth = width;
        arrowX = 1 * pf;
      }
    }

    m_verts[31].m_pos.assign(arrowX, -1 * pf, 0);
    m_verts[32].m_pos.assign(arrowX, -2 * pf, 0);
    m_verts[33].m_pos.assign(arrowX + arrowLineWidth, -1 * pf, 0);
    m_verts[34].m_pos.assign(arrowX + arrowLineWidth, -2 * pf, 0);

    _loadVerts();
  }

  m_nomWidth = width;
  m_nomHeight = height;
}

void Button::setIcon(Icon* icon) {
  if (icon)
    m_icon.reset(new IconView(rootView().viewRes(), *this, *icon));
  else
    m_icon.reset();
  setText(m_textStr);
  updateSize();
}

void Button::colorGlyphs(const zeus::CColor& newColor) {
  m_textColor = newColor;
  m_text->colorGlyphs(newColor);
  for (int i = 28; i < 31; ++i)
    m_verts[i].m_color = newColor;
  _loadVerts();
}

void Button::ButtonTarget::setInactive() {
  if (m_button.m_style == Style::Block) {
    zeus::CColor c1 = rootView().themeData().button1Inactive() * m_button.m_bgColor;
    zeus::CColor c2 = rootView().themeData().button2Inactive() * m_button.m_bgColor;
    m_button.m_verts[0].m_color = c1;
    m_button.m_verts[1].m_color = c2;
    m_button.m_verts[2].m_color = c1;
    m_button.m_verts[3].m_color = c2;
    m_button.m_verts[4].m_color = c2;
    m_button._loadVerts();
  } else {
    for (int i = 0; i < 4; ++i)
      m_button.m_verts[i].m_color = zeus::skClear;
    m_button._loadVerts();
    m_button.m_text->colorGlyphs(m_button.m_textColor);
  }
}

void Button::MenuTarget::setInactive() {
  if (m_button.m_style == Style::Block) {
    zeus::CColor c1 = rootView().themeData().button1Inactive() * m_button.m_bgColor;
    zeus::CColor c2 = rootView().themeData().button2Inactive() * m_button.m_bgColor;
    m_button.m_verts[31].m_color = c1;
    m_button.m_verts[32].m_color = c2;
    m_button.m_verts[33].m_color = c1;
    m_button.m_verts[34].m_color = c2;
    m_button._loadVerts();
  } else {
    for (int i = 28; i < 31; ++i)
      m_button.m_verts[i].m_color = m_button.m_textColor;
    for (int i = 31; i < 35; ++i)
      m_button.m_verts[i].m_color = zeus::skClear;
    m_button._loadVerts();
  }
}

void Button::ButtonTarget::setHover() {
  if (m_button.m_style == Style::Block) {
    zeus::CColor c1 = rootView().themeData().button1Hover() * m_button.m_bgColor;
    zeus::CColor c2 = rootView().themeData().button2Hover() * m_button.m_bgColor;
    m_button.m_verts[0].m_color = c1;
    m_button.m_verts[1].m_color = c2;
    m_button.m_verts[2].m_color = c1;
    m_button.m_verts[3].m_color = c2;
    m_button.m_verts[4].m_color = c2;
    m_button._loadVerts();
  } else {
    for (int i = 0; i < 4; ++i)
      m_button.m_verts[i].m_color = m_button.m_textColor;
    m_button._loadVerts();
    m_button.m_text->colorGlyphs(m_button.m_textColor);
  }
}

void Button::MenuTarget::setHover() {
  if (m_button.m_style == Style::Block) {
    zeus::CColor c1 = rootView().themeData().button1Hover() * m_button.m_bgColor;
    zeus::CColor c2 = rootView().themeData().button2Hover() * m_button.m_bgColor;
    m_button.m_verts[31].m_color = c1;
    m_button.m_verts[32].m_color = c2;
    m_button.m_verts[33].m_color = c1;
    m_button.m_verts[34].m_color = c2;
    m_button._loadVerts();
  } else {
    for (int i = 28; i < 31; ++i)
      m_button.m_verts[i].m_color = m_button.m_textColor;
    for (int i = 31; i < 35; ++i)
      m_button.m_verts[i].m_color = m_button.m_textColor;
    m_button._loadVerts();
  }
}

void Button::ButtonTarget::setPressed() {
  if (m_button.m_style == Style::Block) {
    zeus::CColor c1 = rootView().themeData().button1Press() * m_button.m_bgColor;
    zeus::CColor c2 = rootView().themeData().button2Press() * m_button.m_bgColor;
    m_button.m_verts[0].m_color = c1;
    m_button.m_verts[1].m_color = c2;
    m_button.m_verts[2].m_color = c1;
    m_button.m_verts[3].m_color = c2;
    m_button.m_verts[4].m_color = c2;
    m_button._loadVerts();
  } else {
    for (int i = 0; i < 4; ++i)
      m_button.m_verts[i].m_color = m_button.m_textColor;
    m_button._loadVerts();
    m_button.m_text->colorGlyphs(m_button.m_textColor);
  }
}

void Button::MenuTarget::setPressed() {
  if (m_button.m_style == Style::Block) {
    zeus::CColor c1 = rootView().themeData().button1Press() * m_button.m_bgColor;
    zeus::CColor c2 = rootView().themeData().button2Press() * m_button.m_bgColor;
    m_button.m_verts[31].m_color = c1;
    m_button.m_verts[32].m_color = c2;
    m_button.m_verts[33].m_color = c1;
    m_button.m_verts[34].m_color = c2;
    m_button._loadVerts();
  } else {
    for (int i = 28; i < 31; ++i)
      m_button.m_verts[i].m_color = m_button.m_textColor;
    for (int i = 31; i < 35; ++i)
      m_button.m_verts[i].m_color = m_button.m_textColor;
    m_button._loadVerts();
  }
}

void Button::ButtonTarget::setDisabled() {
  if (m_button.m_style == Style::Block) {
    zeus::CColor c1 = rootView().themeData().button1Disabled() * m_button.m_bgColor;
    zeus::CColor c2 = rootView().themeData().button2Disabled() * m_button.m_bgColor;
    m_button.m_verts[0].m_color = c1;
    m_button.m_verts[1].m_color = c2;
    m_button.m_verts[2].m_color = c1;
    m_button.m_verts[3].m_color = c2;
    m_button.m_verts[4].m_color = c2;
    m_button._loadVerts();
  } else {
    for (int i = 0; i < 4; ++i)
      m_button.m_verts[i].m_color = zeus::skClear;
    m_button._loadVerts();
    zeus::CColor dimText = m_button.m_textColor;
    dimText[3] *= 0.5;
    m_button.m_text->colorGlyphs(dimText);
  }
}

void Button::MenuTarget::setDisabled() {
  if (m_button.m_style == Style::Block) {
    zeus::CColor c1 = rootView().themeData().button1Disabled() * m_button.m_bgColor;
    zeus::CColor c2 = rootView().themeData().button2Disabled() * m_button.m_bgColor;
    m_button.m_verts[31].m_color = c1;
    m_button.m_verts[32].m_color = c2;
    m_button.m_verts[33].m_color = c1;
    m_button.m_verts[34].m_color = c2;
    m_button._loadVerts();
  } else {
    zeus::CColor dimText = m_button.m_textColor;
    dimText[3] *= 0.5;
    for (int i = 28; i < 31; ++i)
      m_button.m_verts[i].m_color = dimText;
    for (int i = 31; i < 35; ++i)
      m_button.m_verts[i].m_color = zeus::skClear;
    m_button._loadVerts();
  }
}

void Button::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod) {
  if (m_menuStyle != IButtonBinding::MenuStyle::Primary)
    m_buttonTarget.mouseDown(coord, button, mod);
  m_menuTarget.mouseDown(coord, button, mod);
}

void Button::ButtonTarget::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod) {
  m_pressed = true;
  setPressed();
  if (m_button.m_controlBinding)
    static_cast<IButtonBinding&>(*m_button.m_controlBinding).down(&m_button, coord);
}

void Button::MenuTarget::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod) {
  m_pressed = true;
  setPressed();
  if (m_hovered) {
    Log.report(logvisor::Info, fmt("button menu '{}' activated"), m_button.m_textStr);
    if (m_button.m_controlBinding) {
      m_button.m_modalMenu.m_view = static_cast<IButtonBinding&>(*m_button.m_controlBinding).buildMenu(&m_button);
      rootView().setActiveMenuButton(&m_button);
      m_button.updateSize();
    }
  }
}

void Button::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod) {
  if (m_menuStyle != IButtonBinding::MenuStyle::Primary)
    m_buttonTarget.mouseUp(coord, button, mod);
  m_menuTarget.mouseUp(coord, button, mod);
}

void Button::ButtonTarget::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod) {
  if (m_pressed) {
    if (m_button.m_controlBinding)
      static_cast<IButtonBinding&>(*m_button.m_controlBinding).up(&m_button, coord);
    if (m_hovered) {
      Log.report(logvisor::Info, fmt("button '{}' activated"), m_button.m_textStr);
      if (m_button.m_controlBinding)
        static_cast<IButtonBinding&>(*m_button.m_controlBinding).activated(&m_button, coord);
    }
    m_pressed = false;
  }
  if (m_hovered)
    setHover();
  else
    setInactive();
}

void Button::MenuTarget::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod) {
  m_pressed = false;
  if (m_hovered)
    setHover();
  else
    setInactive();
}

void Button::mouseMove(const boo::SWindowCoord& coord) {
  if (m_menuStyle != IButtonBinding::MenuStyle::Primary)
    m_buttonTarget.mouseMove(coord);
  m_menuTarget.mouseMove(coord);
}

void Button::ButtonTarget::mouseEnter(const boo::SWindowCoord& coord) {
  m_hovered = true;
  if (m_pressed)
    setPressed();
  else
    setHover();
}

void Button::MenuTarget::mouseEnter(const boo::SWindowCoord& coord) {
  m_hovered = true;
  if (m_pressed)
    setPressed();
  else
    setHover();
}

void Button::mouseLeave(const boo::SWindowCoord& coord) {
  if (m_menuStyle != IButtonBinding::MenuStyle::Primary)
    m_buttonTarget.mouseLeave(coord);
  m_menuTarget.mouseLeave(coord);
}

void Button::ButtonTarget::mouseLeave(const boo::SWindowCoord& coord) {
  m_hovered = false;
  setInactive();
}

void Button::MenuTarget::mouseLeave(const boo::SWindowCoord& coord) {
  m_hovered = false;
  setInactive();
}

void Button::closeMenu(const boo::SWindowCoord& coord) {
  rootView().unsetActiveMenuButton(this);
  m_modalMenu.m_view.reset();
  m_menuTarget.mouseMove(coord);
}

void Button::think() {
  if (m_modalMenu.m_view)
    m_modalMenu.m_view->think();
}

void Button::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub) {
  View::resized(root, sub);
  boo::SWindowRect textRect = sub;
  float pf = rootView().viewRes().pixelFactor();

  if (m_icon) {
    textRect.location[0] += 18 * pf;
    boo::SWindowRect iconRect = sub;
    iconRect.size[0] = 16 * pf;
    iconRect.size[1] = 16 * pf;
    iconRect.location[1] += 2 * pf;
    if (m_style == Style::Block) {
      iconRect.location[0] += 5 * pf;
      iconRect.location[1] += pf;
    }
    m_icon->resized(root, iconRect);
  }

  if (m_style == Style::Block) {
    textRect.location[0] += 6 * pf;
    textRect.location[1] += 7 * pf;
  }
  textRect.location[0] += m_textWidth / 2;
  textRect.size[0] = m_textWidth;
  textRect.size[1] = m_nomHeight;
  m_text->resized(root, textRect);

  if (m_style == Style::Block) {
    if (m_menuStyle == IButtonBinding::MenuStyle::None) {
      m_buttonTarget.m_view->resized(root, sub);
    } else if (m_menuStyle == IButtonBinding::MenuStyle::Primary) {
      m_menuTarget.m_view->resized(root, sub);
    } else {
      boo::SWindowRect targetRect = sub;
      targetRect.size[0] = m_textIconWidth;
      m_buttonTarget.m_view->resized(root, targetRect);
      targetRect.location[0] += targetRect.size[0];
      targetRect.size[0] = 16 * pf;
      m_menuTarget.m_view->resized(root, targetRect);
    }
  } else {
    if (m_menuStyle == IButtonBinding::MenuStyle::Primary) {
      boo::SWindowRect targetRect = sub;
      targetRect.size[0] = m_nomWidth;
      targetRect.size[1] = m_nomHeight;
      m_menuTarget.m_view->resized(root, targetRect);
    } else {
      boo::SWindowRect targetRect = sub;
      targetRect.size[0] = m_textIconWidth + 3 * pf;
      targetRect.size[1] = m_nomHeight;
      m_buttonTarget.m_view->resized(root, targetRect);
      targetRect.location[0] += targetRect.size[0];
      targetRect.size[0] = 15 * pf;
      m_menuTarget.m_view->resized(root, targetRect);
    }
  }

  if (m_modalMenu.m_view) {
    boo::SWindowRect menuRect = sub;
    if (m_style == Style::Text)
      menuRect.location[1] -= 6 * pf;
    m_modalMenu.m_view->resized(root, menuRect);
  }
}

void Button::draw(boo::IGraphicsCommandQueue* gfxQ) {
  View::draw(gfxQ);
  gfxQ->setShaderDataBinding(m_vertsBinding);
  if (m_style == Style::Block) {
    gfxQ->draw(0, 28);
    if (m_menuStyle != IButtonBinding::MenuStyle::None) {
      gfxQ->draw(31, 4);
      gfxQ->draw(28, 3);
      if (m_menuStyle == IButtonBinding::MenuStyle::Auxiliary)
        gfxQ->draw(35, 4);
    }
  } else {
    gfxQ->draw(0, 4);
    if (m_menuStyle != IButtonBinding::MenuStyle::None) {
      gfxQ->draw(28, 3);
      gfxQ->draw(31, 4);
    }
  }

  if (m_icon)
    m_icon->draw(gfxQ);
  if (m_textStr.size())
    m_text->draw(gfxQ);

  if (m_modalMenu.m_view)
    m_modalMenu.m_view->draw(gfxQ);
}

} // namespace specter
