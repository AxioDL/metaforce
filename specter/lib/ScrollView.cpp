#include "Specter/ScrollView.hpp"
#include "Specter/ViewResources.hpp"
#include "Specter/RootView.hpp"

namespace Specter
{
#define MAX_SCROLL_SPEED 100

ScrollView::ScrollView(ViewResources& res, View& parentView, Style style)
: View(res, parentView), m_style(style)
{
    m_vertsBuf = res.m_factory->newDynamicBuffer(boo::BufferUse::Vertex, sizeof(SolidShaderVert), 4);

    if (!res.m_viewRes.m_texVtxFmt)
    {
        boo::VertexElementDescriptor vdescs[] =
        {
            {m_vertsBuf, nullptr, boo::VertexSemantic::Position4},
            {m_vertsBuf, nullptr, boo::VertexSemantic::Color}
        };
        m_vtxFmt = res.m_factory->newVertexFormat(2, vdescs);
        boo::IGraphicsBuffer* bufs[] = {m_viewVertBlockBuf};
        m_shaderBinding = res.m_factory->newShaderDataBinding(res.m_viewRes.m_solidShader,
                                                              m_vtxFmt, m_vertsBuf, nullptr,
                                                              nullptr, 1, bufs, 0, nullptr);
    }
    else
    {
        boo::IGraphicsBuffer* bufs[] = {m_viewVertBlockBuf};
        m_shaderBinding = res.m_factory->newShaderDataBinding(res.m_viewRes.m_solidShader,
                                                              res.m_viewRes.m_texVtxFmt,
                                                              m_vertsBuf, nullptr,
                                                              nullptr, 1, bufs, 0, nullptr);
    }
    commitResources(res);
}

bool ScrollView::_scroll(const boo::SScrollDelta& scroll)
{
    if (m_contentView)
    {
        float ratio = subRect().size[1] / float(m_contentView->nominalHeight());
        if (ratio >= 1.f)
        {
            m_scroll[0] = 0;
            m_scroll[1] = 0;
            m_targetScroll[0] = 0;
            m_targetScroll[1] = 0;
            return true;
        }

        float pf = rootView().viewRes().pixelFactor();
        double mult = 20.0 * pf;
        if (scroll.isFine)
            mult = 1.0 * pf;
        //m_targetScroll[0] -= scroll.delta[0] * mult;
        m_targetScroll[1] -= scroll.delta[1] * mult;

        m_targetScroll[1] = std::max(m_targetScroll[1], 0);
        int scrollHeight = m_contentView->nominalHeight() - subRect().size[1];
        m_targetScroll[1] = std::min(m_targetScroll[1], scrollHeight);

        if (scroll.isFine)
        {
            m_scroll[0] = m_targetScroll[0];
            m_scroll[1] = m_targetScroll[1];
            return true;
        }
    }
    else
    {
        m_scroll[0] = 0;
        m_scroll[1] = 0;
        m_targetScroll[0] = 0;
        m_targetScroll[1] = 0;
        return true;
    }
    return false;
}

void ScrollView::scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll)
{
    if (_scroll(scroll))
        updateSize();
}

void ScrollView::think()
{
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
    if (m_contentView)
    {
        boo::SWindowRect cRect = sub;
        cRect.location[0] += m_scroll[0];
        cRect.location[1] += sub.size[1] - m_contentView->nominalHeight() + m_scroll[1];
        cRect.size[0] = m_contentView->nominalWidth();
        cRect.size[1] = m_contentView->nominalHeight();
        m_contentView->resized(root, cRect, sub);

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
                const Zeus::CColor& color = rootView().themeData().scrollIndicator();
                for (int i=0 ; i<4 ; ++i)
                    m_verts[i].m_color = color;
                m_vertsBuf->load(m_verts, sizeof(m_verts));
            }
        }
    }
}

void ScrollView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    if (m_contentView)
    {
        m_contentView->draw(gfxQ);

        if (m_style == Style::ThinIndicator && m_drawInd)
        {
            gfxQ->setShaderDataBinding(m_shaderBinding);
            gfxQ->setDrawPrimitive(boo::Primitive::TriStrips);
            gfxQ->draw(0, 4);
        }
    }
}

}
