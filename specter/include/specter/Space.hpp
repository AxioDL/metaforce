#pragma once

#include <memory>

#include "specter/SplitView.hpp"
#include "specter/Toolbar.hpp"
#include "specter/View.hpp"

namespace specter {
class ViewResources;

struct ISplitSpaceController;

struct ISpaceController {
  virtual bool spaceSplitAllowed() const { return false; }
  virtual ISplitSpaceController* spaceSplit(SplitView::Axis axis, int thisSlot) { return nullptr; }
};

struct ISplitSpaceController {
  virtual SplitView* splitView() = 0;
  virtual void updateSplit(float split) = 0;
  virtual void joinViews(SplitView* thisSplit, int thisSlot, SplitView* otherSplit, int otherSlot) = 0;
};

class Space : public View {
  struct CornerView;
  friend class RootView;
  friend struct CornerView;

  ISpaceController& m_controller;
  Toolbar::Position m_tbPos;
  ViewChild<std::unique_ptr<Toolbar>> m_toolbar;
  ViewChild<View*> m_contentView;

  bool m_cornerDrag = false;
  int m_cornerDragPoint[2];

  ViewChild<std::unique_ptr<CornerView>> m_cornerView;

public:
  Space(ViewResources& res, View& parentView, ISpaceController& controller, Toolbar::Position toolbarPos,
        unsigned tbUnits);
  ~Space() override;

  View* setContentView(View* view);
  Toolbar* toolbar() { return m_toolbar.m_view.get(); }
  void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
  void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
  void mouseMove(const boo::SWindowCoord&) override;
  void mouseEnter(const boo::SWindowCoord&) override;
  void mouseLeave(const boo::SWindowCoord&) override;
  void resized(const boo::SWindowRect& rootView, const boo::SWindowRect& sub) override;
  void draw(boo::IGraphicsCommandQueue* gfxQ) override;

  SplitView* findSplitViewOnSide(SplitView::Axis axis, int side);

  void setMultiplyColor(const zeus::CColor& color) override {
    View::setMultiplyColor(color);
    if (m_contentView.m_view)
      m_contentView.m_view->setMultiplyColor(color);
    if (m_toolbar.m_view)
      m_toolbar.m_view->setMultiplyColor(color);
  }

  bool isSpace() const override { return true; }
};
inline Space* View::castToSpace() { return isSpace() ? static_cast<Space*>(this) : nullptr; }

} // namespace specter
