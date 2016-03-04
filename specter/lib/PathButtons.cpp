#include "specter/PathButtons.hpp"
#include "specter/RootView.hpp"
#include "specter/ViewResources.hpp"

namespace specter
{

PathButtons::PathButtons(ViewResources& res, View& parentView, IPathButtonsBinding& binding, bool fillContainer)
: ScrollView(res, parentView, ScrollView::Style::SideButtons), m_binding(binding), m_fillContainer(fillContainer)
{
    m_contentView.m_view.reset(new ContentView(res, *this));
    setContentView(m_contentView.m_view.get());
}

void PathButtons::setButtons(const std::vector<hecl::SystemString>& comps)
{
    m_pathButtons.clear();
    m_pathButtons.reserve(comps.size());
    size_t idx = 0;
    ViewResources& res = rootView().viewRes();
    for (const hecl::SystemString& c : comps)
        m_pathButtons.emplace_back(*this, res, idx++, c);
}

void PathButtons::setMultiplyColor(const zeus::CColor& color)
{
    ScrollView::setMultiplyColor(color);
    for (PathButton& b : m_pathButtons)
        b.m_button.m_view->setMultiplyColor(color);
}

void PathButtons::ContentView::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    for (PathButton& b : m_pb.m_pathButtons)
        b.m_button.mouseDown(coord, button, mod);
}

void PathButtons::ContentView::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    for (PathButton& b : m_pb.m_pathButtons)
        b.m_button.mouseUp(coord, button, mod);
    if (m_pb.m_pathButtonPending >= 0)
    {
        m_pb.m_binding.pathButtonActivated(m_pb.m_pathButtonPending);
        m_pb.m_pathButtonPending = -1;
    }
}

void PathButtons::ContentView::mouseMove(const boo::SWindowCoord& coord)
{
    for (PathButton& b : m_pb.m_pathButtons)
        b.m_button.mouseMove(coord);
}

void PathButtons::ContentView::mouseLeave(const boo::SWindowCoord& coord)
{
    for (PathButton& b : m_pb.m_pathButtons)
        b.m_button.mouseLeave(coord);
}

void PathButtons::ContentView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub,
                                       const boo::SWindowRect& scissor)
{
    View::resized(root, sub);
    m_scissorRect = scissor;
    m_scissorRect.size[1] += 2;
    boo::SWindowRect pathRect = sub;
    for (PathButton& b : m_pb.m_pathButtons)
    {
        pathRect.size[0] = b.m_button.m_view->nominalWidth();
        pathRect.size[1] = b.m_button.m_view->nominalHeight();
        b.m_button.m_view->resized(root, pathRect);
        pathRect.location[0] += pathRect.size[0] + 2;
    }
}

void PathButtons::containerResized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    if (m_fillContainer)
    {
        boo::SWindowRect fillRect = sub;
        fillRect.size[1] = 20 * rootView().viewRes().pixelFactor();
        View::resized(root, fillRect);
    }
}

void PathButtons::ContentView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    gfxQ->setScissor(m_scissorRect);
    for (PathButton& b : m_pb.m_pathButtons)
        b.m_button.m_view->draw(gfxQ);
    gfxQ->setScissor(rootView().subRect());
}

}
