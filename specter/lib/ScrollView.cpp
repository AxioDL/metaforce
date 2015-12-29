#include "Specter/ScrollView.hpp"

namespace Specter
{
#define MAX_SCROLL_SPEED 100

ScrollView::ScrollView(ViewResources& res, View& parentView)
: View(res, parentView)
{
    commitResources(res);
}

void ScrollView::scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll)
{
    if (m_contentView)
    {
        double mult = 10.0;
        if (scroll.isFine)
            mult = 1.0;
        m_targetScroll[0] += scroll.delta[0] * mult;
        m_targetScroll[1] += scroll.delta[1] * mult;
        m_targetScroll[1] = std::min(m_targetScroll[1], 0);
        if (scroll.isFine)
        {
            m_scroll[0] = m_targetScroll[0];
            m_scroll[1] = m_targetScroll[1];
            updateSize();
        }
    }
    else
    {
        m_scroll[0] = 0;
        m_scroll[1] = 0;
        m_targetScroll[0] = 0;
        m_targetScroll[1] = 0;
        updateSize();
    }
}

void ScrollView::think()
{
    bool update = false;

    int xSpeed = std::max(1, std::min(abs(m_targetScroll[0] - m_scroll[0]) / 10, MAX_SCROLL_SPEED));
    if (m_scroll[0] < m_targetScroll[0])
    {
        m_scroll[0] += xSpeed;
        update = true;
    }
    else if (m_scroll[0] > m_targetScroll[0])
    {
        m_scroll[0] -= xSpeed;
        update = true;
    }

    int ySpeed = std::max(1, std::min(abs(m_targetScroll[1] - m_scroll[1]) / 10, MAX_SCROLL_SPEED));
    if (m_scroll[1] < m_targetScroll[1])
    {
        m_scroll[1] += ySpeed;
        update = true;
    }
    else if (m_scroll[1] > m_targetScroll[1])
    {
        m_scroll[1] -= ySpeed;
        update = true;
    }
    if (update)
        updateSize();
}

void ScrollView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);
    if (m_contentView)
    {
        boo::SWindowRect cRect = sub;
        cRect.location[0] += m_scroll[0];
        cRect.location[1] += sub.size[1] - m_contentView->nominalHeight() - m_scroll[1];
        cRect.size[0] = m_contentView->nominalWidth();
        cRect.size[1] = m_contentView->nominalHeight();
        m_contentView->resized(root, cRect, sub);
    }
}

void ScrollView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    if (m_contentView)
        m_contentView->draw(gfxQ);
}

}
