#include "specter/PathButtons.hpp"

#include "specter/Button.hpp"
#include "specter/RootView.hpp"
#include "specter/ViewResources.hpp"

#include <boo/graphicsdev/IGraphicsCommandQueue.hpp>

namespace specter {
struct PathButtons::PathButton final : IButtonBinding {
  PathButtons& m_pb;
  size_t m_idx;
  ViewChild<std::unique_ptr<Button>> m_button;

  PathButton(PathButtons& pb, ViewResources& res, size_t idx, const hecl::SystemString& str) : m_pb(pb), m_idx(idx) {
    m_button.m_view = std::make_unique<Button>(res, pb, this, hecl::SystemUTF8Conv(str).str());
  }

  std::string_view name(const Control* control) const override { return m_button.m_view->getText(); }
  void activated(const Button* button, const boo::SWindowCoord&) override { m_pb.m_pathButtonPending = m_idx; }
};

struct PathButtons::ContentView : public View {
  PathButtons& m_pb;
  boo::SWindowRect m_scissorRect;

  ContentView(ViewResources& res, PathButtons& pb) : View(res, pb), m_pb(pb) {}

  void mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod) override {
    for (PathButton& b : m_pb.m_pathButtons) {
      b.m_button.mouseDown(coord, button, mod);
    }
  }

  void mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod) override {
    for (PathButton& b : m_pb.m_pathButtons) {
      b.m_button.mouseUp(coord, button, mod);
    }

    if (m_pb.m_pathButtonPending >= 0) {
      m_pb.m_binding.pathButtonActivated(m_pb.m_pathButtonPending);
      m_pb.m_pathButtonPending = -1;
    }
  }

  void mouseMove(const boo::SWindowCoord& coord) override {
    for (PathButton& b : m_pb.m_pathButtons) {
      b.m_button.mouseMove(coord);
    }
  }

  void mouseLeave(const boo::SWindowCoord& coord) override {
    for (PathButton& b : m_pb.m_pathButtons) {
      b.m_button.mouseLeave(coord);
    }
  }

  int nominalWidth() const override {
    int ret = 0;
    for (const PathButton& b : m_pb.m_pathButtons) {
      ret += b.m_button.m_view->nominalWidth() + 2;
    }
    return ret;
  }

  int nominalHeight() const override {
    return m_pb.m_pathButtons.size() ? m_pb.m_pathButtons[0].m_button.m_view->nominalHeight() : 0;
  }

  void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub, const boo::SWindowRect& scissor) override {
    View::resized(root, sub);

    m_scissorRect = scissor;
    m_scissorRect.size[1] += 2;

    boo::SWindowRect pathRect = sub;
    for (PathButton& b : m_pb.m_pathButtons) {
      pathRect.size[0] = b.m_button.m_view->nominalWidth();
      pathRect.size[1] = b.m_button.m_view->nominalHeight();
      b.m_button.m_view->resized(root, pathRect);
      pathRect.location[0] += pathRect.size[0] + 2;
    }
  }

  void draw(boo::IGraphicsCommandQueue* gfxQ) override {
    gfxQ->setScissor(m_scissorRect);

    for (PathButton& b : m_pb.m_pathButtons) {
      b.m_button.m_view->draw(gfxQ);
    }

    gfxQ->setScissor(rootView().subRect());
  }
};

PathButtons::PathButtons(ViewResources& res, View& parentView, IPathButtonsBinding& binding, bool fillContainer)
: ScrollView(res, parentView, ScrollView::Style::SideButtons), m_binding(binding), m_fillContainer(fillContainer) {
  m_contentView.m_view = std::make_unique<ContentView>(res, *this);
  setContentView(m_contentView.m_view.get());
}

PathButtons::~PathButtons() = default;

void PathButtons::setButtons(const std::vector<hecl::SystemString>& comps) {
  m_pathButtons.clear();
  m_pathButtons.reserve(comps.size());
  size_t idx = 0;
  ViewResources& res = rootView().viewRes();
  for (const hecl::SystemString& c : comps)
    m_pathButtons.emplace_back(*this, res, idx++, c);
}

void PathButtons::setMultiplyColor(const zeus::CColor& color) {
  ScrollView::setMultiplyColor(color);
  for (PathButton& b : m_pathButtons)
    b.m_button.m_view->setMultiplyColor(color);
}

void PathButtons::containerResized(const boo::SWindowRect& root, const boo::SWindowRect& sub) {
  if (m_fillContainer) {
    boo::SWindowRect fillRect = sub;
    fillRect.size[1] = 20 * rootView().viewRes().pixelFactor();
    View::resized(root, fillRect);
  }
}

} // namespace specter
