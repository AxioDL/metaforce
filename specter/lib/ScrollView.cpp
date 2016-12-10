#include "specter/ScrollView.hpp"
#include "specter/ViewResources.hpp"
#include "specter/RootView.hpp"
#include "specter/Button.hpp"

namespace specter
{
#define MAX_SCROLL_SPEED 100

ScrollView::ScrollView(ViewResources& res, View& parentView, Style style)
: View(res, parentView), m_style(style), m_sideButtonBind(*this, rootView().viewManager())
{
    commitResources(res, [&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        buildResources(ctx, res);
        m_vertsBinding.init(ctx, res, 4, *m_viewVertBlockBuf);
        return true;
    });

    if (style == Style::SideButtons)
    {
        m_sideButtons[0].m_view.reset(new Button(res, *this, &m_sideButtonBind, "<"));
        m_sideButtons[1].m_view.reset(new Button(res, *this, &m_sideButtonBind, ">"));
    }
}

bool ScrollView::_scroll(const boo::SScrollDelta& scroll)
{
    if (m_contentView.m_view)
    {
        float ratioX = subRect().size[0] / float(m_contentView.m_view->nominalWidth());
        float ratioY = subRect().size[1] / float(m_contentView.m_view->nominalHeight());

        float pf = rootView().viewRes().pixelFactor();
        double mult = 20.0 * pf;
        if (scroll.isFine)
            mult = 1.0 * pf;

        bool ret = false;

        if (ratioX >= 1.f)
        {
            m_scroll[0] = 0;
            m_targetScroll[0] = 0;
            m_drawSideButtons = false;
            ret = true;
        }
        else
        {
            m_drawSideButtons = true;
            m_targetScroll[0] += scroll.delta[0] * mult;
            m_targetScroll[0] = std::min(m_targetScroll[0], 0);
            int scrollWidth = m_contentView.m_view->nominalWidth() - scrollAreaWidth();
            m_targetScroll[0] = std::max(m_targetScroll[0], -scrollWidth);
        }

        if (ratioY >= 1.f)
        {
            m_scroll[1] = 0;
            m_targetScroll[1] = 0;
            ret = true;
        }
        else
        {
            m_targetScroll[1] -= scroll.delta[1] * mult;
            m_targetScroll[1] = std::max(m_targetScroll[1], 0);
            int scrollHeight = m_contentView.m_view->nominalHeight() - subRect().size[1];
            m_targetScroll[1] = std::min(m_targetScroll[1], scrollHeight);
        }

        if (scroll.isFine)
        {
            m_scroll[0] = m_targetScroll[0];
            m_scroll[1] = m_targetScroll[1];
            ret = true;
        }

        return ret;
    }
    else
    {
        m_scroll[0] = 0;
        m_scroll[1] = 0;
        m_targetScroll[0] = 0;
        m_targetScroll[1] = 0;
        m_drawSideButtons = false;
        return true;
    }
    return false;
}

int ScrollView::scrollAreaWidth() const
{
    int ret = subRect().size[0];
    if (m_style == Style::SideButtons && m_drawSideButtons)
    {
        ret -= m_sideButtons[0].m_view->nominalWidth();
        ret -= m_sideButtons[1].m_view->nominalWidth();
    }
    return std::max(0, ret);
}

void ScrollView::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (m_style == Style::SideButtons && m_drawSideButtons)
    {
        if (m_sideButtons[0].mouseDown(coord, button, mod) ||
            m_sideButtons[1].mouseDown(coord, button, mod))
            return;
    }
    m_contentView.mouseDown(coord, button, mod);
}

void ScrollView::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (m_style == Style::SideButtons)
    {
        m_sideButtons[0].mouseUp(coord, button, mod);
        m_sideButtons[1].mouseUp(coord, button, mod);
    }
    m_contentView.mouseUp(coord, button, mod);
}

void ScrollView::mouseMove(const boo::SWindowCoord& coord)
{
    if (m_style == Style::SideButtons && m_drawSideButtons)
    {
        m_sideButtons[0].mouseMove(coord);
        m_sideButtons[1].mouseMove(coord);
    }
    m_contentView.mouseMove(coord);
}

void ScrollView::mouseEnter(const boo::SWindowCoord& coord)
{
    if (m_style == Style::SideButtons && m_drawSideButtons)
    {
        m_sideButtons[0].mouseEnter(coord);
        m_sideButtons[1].mouseEnter(coord);
    }
    m_contentView.mouseEnter(coord);
}

void ScrollView::mouseLeave(const boo::SWindowCoord& coord)
{
    if (m_style == Style::SideButtons)
    {
        m_sideButtons[0].mouseLeave(coord);
        m_sideButtons[1].mouseLeave(coord);
    }
    m_contentView.mouseLeave(coord);
}

void ScrollView::scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll)
{
    if (!scroll.isAccelerated)
    {
        boo::SScrollDelta newScroll = scroll;
        m_consecutiveScroll[m_consecutiveIdx][0] += scroll.delta[0];
        m_consecutiveScroll[m_consecutiveIdx][1] += scroll.delta[1];
        newScroll.delta[0] = 0;
        newScroll.delta[1] = 0;
        for (size_t i=0 ; i<16 ; ++i)
        {
            newScroll.delta[0] += m_consecutiveScroll[i][0];
            newScroll.delta[1] += m_consecutiveScroll[i][1];
        }
        if (_scroll(newScroll))
            updateSize();
        return;
    }
    if (_scroll(scroll))
        updateSize();
}

void ScrollView::setMultiplyColor(const zeus::CColor& color)
{
    View::setMultiplyColor(color);
    if (m_style == Style::SideButtons)
    {
        m_sideButtons[0].m_view->setMultiplyColor(color);
        m_sideButtons[1].m_view->setMultiplyColor(color);
    }
    if (m_contentView.m_view)
        m_contentView.m_view->setMultiplyColor(color);
}

void ScrollView::think()
{
    m_consecutiveIdx = (m_consecutiveIdx+1) % 16;
    m_consecutiveScroll[m_consecutiveIdx][0] = 0.0;
    m_consecutiveScroll[m_consecutiveIdx][1] = 0.0;

    if (m_sideButtonState != SideButtonState::None)
    {
        if (m_sideButtonState == SideButtonState::ScrollLeft)
            m_targetScroll[0] -= 3;
        else if (m_sideButtonState == SideButtonState::ScrollRight)
            m_targetScroll[0] += 3;
        m_targetScroll[0] = std::min(m_targetScroll[0], 0);
        int scrollWidth = m_contentView.m_view->nominalWidth() - scrollAreaWidth();
        m_targetScroll[0] = std::max(m_targetScroll[0], -scrollWidth);
    }

    bool update = false;
    float pf = rootView().viewRes().pixelFactor();

    int xSpeed = std::max(1, std::min(abs(m_targetScroll[0] - m_scroll[0]) / int(5*pf), int(pf*MAX_SCROLL_SPEED)));
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

    int ySpeed = std::max(1, std::min(abs(m_targetScroll[1] - m_scroll[1]) / int(5*pf), int(pf*MAX_SCROLL_SPEED)));
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
    _scroll({});
    if (m_contentView.m_view)
    {
        boo::SWindowRect cRect = sub;
        cRect.location[0] += m_scroll[0];
        cRect.location[1] += sub.size[1] - m_contentView.m_view->nominalHeight() + m_scroll[1];
        cRect.size[0] = m_contentView.m_view->nominalWidth();
        cRect.size[1] = m_contentView.m_view->nominalHeight();
        m_contentView.m_scissorRect = sub;
        if (m_style == Style::SideButtons && m_drawSideButtons)
        {
            int width0 = m_sideButtons[0].m_view->nominalWidth() + 2;
            int width1 = m_sideButtons[1].m_view->nominalWidth();
            cRect.location[0] += width0;
            cRect.size[0] -= (width0 + width1);

            m_contentView.m_scissorRect.location[0] += width0;
            m_contentView.m_scissorRect.size[0] -= (width0 + width1);
        }
        m_contentView.m_view->resized(root, cRect, m_contentView.m_scissorRect);


        if (m_style == Style::ThinIndicator)
        {
            float ratio = sub.size[1] / float(cRect.size[1]);
            m_drawInd = ratio < 1.f;
            if (m_drawInd)
            {
                float pf = rootView().viewRes().pixelFactor();
                int barHeight = sub.size[1] * ratio;
                int scrollHeight = sub.size[1] - barHeight;
                float prog = m_scroll[1] / float(cRect.size[1] - sub.size[1]);
                int x = sub.size[0];
                int y = sub.size[1] - scrollHeight * prog;
                m_verts[0].m_pos.assign(x, y, 0);
                m_verts[1].m_pos.assign(x, y-barHeight, 0);
                m_verts[2].m_pos.assign(x+2*pf, y, 0);
                m_verts[3].m_pos.assign(x+2*pf, y-barHeight, 0);
                const zeus::CColor& color = rootView().themeData().scrollIndicator();
                for (int i=0 ; i<4 ; ++i)
                    m_verts[i].m_color = color;
                m_vertsBinding.load<decltype(m_verts)>(m_verts);
            }
        }
        else if (m_style == Style::SideButtons && m_drawSideButtons)
        {
            boo::SWindowRect bRect = sub;
            bRect.size[0] = m_sideButtons[0].m_view->nominalWidth();
            bRect.size[1] = m_sideButtons[0].m_view->nominalHeight();
            m_sideButtons[0].m_view->resized(root, bRect);

            bRect.size[0] = m_sideButtons[1].m_view->nominalWidth();
            bRect.size[1] = m_sideButtons[1].m_view->nominalHeight();
            bRect.location[0] += sub.size[0] - bRect.size[0];
            m_sideButtons[1].m_view->resized(root, bRect);
        }
    }
}

void ScrollView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    if (m_contentView.m_view)
    {
        m_contentView.m_view->draw(gfxQ);

        if (m_style == Style::ThinIndicator && m_drawInd)
        {
            gfxQ->setShaderDataBinding(m_vertsBinding);
            gfxQ->draw(0, 4);
        }
        else if (m_style == Style::SideButtons && m_drawSideButtons)
        {
            m_sideButtons[0].m_view->draw(gfxQ);
            m_sideButtons[1].m_view->draw(gfxQ);
        }
    }
}

}
