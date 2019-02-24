#include "specter/MessageWindow.hpp"
#include "specter/ViewResources.hpp"
#include "specter/RootView.hpp"
#include "specter/Menu.hpp"

namespace specter {

MessageWindow::MessageWindow(ViewResources& res, View& parentView, Type type, std::string_view message,
                             std::function<void(bool)> func)
: ModalWindow(res, parentView, RectangleConstraint(),
              type == Type::ErrorOk ? res.themeData().splashErrorBackground() : res.themeData().splashBackground())
, m_type(type)
, m_func(func)
, m_okBind(*this, rootView().viewManager().translateOr("ok", "OK"))
, m_cancelBind(*this, rootView().viewManager().translateOr("cancel", "Cancel")) {
  m_text.reset(new MultiLineTextView(res, *this, res.m_mainFont, TextView::Alignment::Center));
  m_text->typesetGlyphs(message, res.themeData().uiText(), 380 * res.pixelFactor());
  constraint() = RectangleConstraint(400 * res.pixelFactor(), 80 * res.pixelFactor() + m_text->nominalHeight());

  m_ok.m_view.reset(new Button(res, *this, &m_okBind, m_okBind.m_name, nullptr, Button::Style::Block,
                               zeus::skWhite, RectangleConstraint(150 * res.pixelFactor())));
  if (type == Type::ConfirmOkCancel)
    m_cancel.m_view.reset(new Button(res, *this, &m_cancelBind, m_cancelBind.m_name, nullptr, Button::Style::Block,
                                     zeus::skWhite, RectangleConstraint(150 * res.pixelFactor())));

  updateContentOpacity(0.0);
}

void MessageWindow::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods) {
  if (closed() || skipBuildInAnimation())
    return;
  m_ok.mouseDown(coord, button, mods);
  m_cancel.mouseDown(coord, button, mods);
}

void MessageWindow::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods) {
  if (closed())
    return;
  m_ok.mouseUp(coord, button, mods);
  m_cancel.mouseUp(coord, button, mods);
}

void MessageWindow::mouseMove(const boo::SWindowCoord& coord) {
  if (closed())
    return;
  m_ok.mouseMove(coord);
  m_cancel.mouseMove(coord);
}

void MessageWindow::mouseEnter(const boo::SWindowCoord& coord) {
  if (closed())
    return;
  m_ok.mouseEnter(coord);
  m_cancel.mouseEnter(coord);
}

void MessageWindow::mouseLeave(const boo::SWindowCoord& coord) {
  if (closed())
    return;
  m_ok.mouseLeave(coord);
  m_cancel.mouseLeave(coord);
}

void MessageWindow::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub) {
  ModalWindow::resized(root, sub);
  boo::SWindowRect buttonRect = subRect();
  float pf = rootView().viewRes().pixelFactor();
  buttonRect.location[1] += 20 * pf;
  buttonRect.size[0] = m_ok.m_view->nominalWidth();
  buttonRect.size[1] = m_ok.m_view->nominalHeight();
  if (m_type == Type::ConfirmOkCancel) {
    buttonRect.location[0] += 45 * pf;
    m_ok.m_view->resized(root, buttonRect);
    buttonRect.location[0] += 160 * pf;
    m_cancel.m_view->resized(root, buttonRect);
  } else {
    buttonRect.location[0] += 125 * pf;
    m_ok.m_view->resized(root, buttonRect);
  }

  boo::SWindowRect textRect = subRect();
  textRect.location[0] += 200 * pf;
  textRect.location[1] += 65 * pf;
  m_text->resized(root, textRect);
}

void MessageWindow::draw(boo::IGraphicsCommandQueue* gfxQ) {
  ModalWindow::draw(gfxQ);
  m_text->draw(gfxQ);
  m_ok.m_view->draw(gfxQ);
  if (m_type == Type::ConfirmOkCancel)
    m_cancel.m_view->draw(gfxQ);
}

} // namespace specter
