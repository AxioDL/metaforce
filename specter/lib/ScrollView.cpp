#include "Specter/ScrollView.hpp"

namespace Specter
{

ScrollView::ScrollView(ViewResources& res, View& parentView)
: View(res, parentView)
{
    commitResources(res);
}

void ScrollView::scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll)
{
    m_scroll[0] += scroll.delta[0];
    m_scroll[1] += scroll.delta[1];
    updateSize();
}

void ScrollView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);
    if (m_contentView)
    {
        boo::SWindowRect cRect = sub;
        cRect.location[0] += m_scroll[0];
        cRect.location[1] += sub.size[1] - m_contentView->nominalHeight() + m_scroll[1];
        m_contentView->resized(root, cRect, sub);
    }
}

void ScrollView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    if (m_contentView)
        m_contentView->draw(gfxQ);
}

}
