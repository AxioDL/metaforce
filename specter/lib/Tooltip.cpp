#include "Specter/Tooltip.hpp"
#include "Specter/ViewResources.hpp"
#include "Specter/RootView.hpp"

namespace Specter
{

#define TOOLTIP_MAX_WIDTH 316
#define TOOLTIP_MAX_TEXT_WIDTH 300
#define TOOLTIP_MARGIN 8

Tooltip::Tooltip(ViewResources& res, View& parentView, const std::string& title,
                 const std::string& message)
: View(res, parentView), m_titleStr(title), m_messageStr(message)
{
    m_ttBlockBuf = res.m_factory->newDynamicBuffer(boo::BufferUse::Uniform, sizeof(ViewBlock), 1);
    m_ttVertsBuf = res.m_factory->newDynamicBuffer(boo::BufferUse::Vertex, sizeof(SolidShaderVert), 16);

    if (!res.m_viewRes.m_solidVtxFmt)
    {
        boo::VertexElementDescriptor vdescs[] =
        {
            {m_ttVertsBuf, nullptr, boo::VertexSemantic::Position4},
            {m_ttVertsBuf, nullptr, boo::VertexSemantic::Color}
        };
        m_ttVtxFmt = res.m_factory->newVertexFormat(2, vdescs);
        boo::IGraphicsBuffer* bufs[] = {m_ttBlockBuf};
        m_ttShaderBinding = res.m_factory->newShaderDataBinding(res.m_viewRes.m_solidShader,
                                                                m_ttVtxFmt, m_ttVertsBuf, nullptr,
                                                                nullptr, 1, bufs, 0, nullptr);
    }
    else
    {
        boo::IGraphicsBuffer* bufs[] = {m_ttBlockBuf};
        m_ttShaderBinding = res.m_factory->newShaderDataBinding(res.m_viewRes.m_solidShader,
                                                                res.m_viewRes.m_solidVtxFmt,
                                                                m_ttVertsBuf, nullptr,
                                                                nullptr, 1, bufs, 0, nullptr);
    }

    for (int i=0 ; i<16 ; ++i)
        m_ttVerts[i].m_color = res.themeData().tooltipBackground();

    commitResources(res);
    resetResources(res);
}

#define EDGE_EPSILON 0.25

void Tooltip::setVerts(int width, int height, float pf)
{
    int margin = TOOLTIP_MARGIN * pf;
    width = std::max(width, margin*2);
    height = std::max(height, margin*2);

    m_ttVerts[0].m_pos.assign(0, height-margin-EDGE_EPSILON, 0);
    m_ttVerts[1].m_pos.assign(0, margin+EDGE_EPSILON, 0);
    m_ttVerts[2].m_pos.assign(width, height-margin-EDGE_EPSILON, 0);
    m_ttVerts[3].m_pos.assign(width, margin+EDGE_EPSILON, 0);
    m_ttVerts[4].m_pos.assign(width, margin+EDGE_EPSILON, 0);

    m_ttVerts[5].m_pos.assign(margin+EDGE_EPSILON, height, 0);
    m_ttVerts[6].m_pos.assign(margin+EDGE_EPSILON, height, 0);
    m_ttVerts[7].m_pos.assign(margin+EDGE_EPSILON, height-margin+EDGE_EPSILON, 0);
    m_ttVerts[8].m_pos.assign(width-margin-EDGE_EPSILON, height, 0);
    m_ttVerts[9].m_pos.assign(width-margin-EDGE_EPSILON, height-margin+EDGE_EPSILON, 0);
    m_ttVerts[10].m_pos.assign(width-margin-EDGE_EPSILON, height-margin+EDGE_EPSILON, 0);

    m_ttVerts[11].m_pos.assign(margin+EDGE_EPSILON, margin-EDGE_EPSILON, 0);
    m_ttVerts[12].m_pos.assign(margin+EDGE_EPSILON, margin-EDGE_EPSILON, 0);
    m_ttVerts[13].m_pos.assign(margin+EDGE_EPSILON, 0, 0);
    m_ttVerts[14].m_pos.assign(width-margin-EDGE_EPSILON, margin-EDGE_EPSILON, 0);
    m_ttVerts[15].m_pos.assign(width-margin-EDGE_EPSILON, 0, 0);

    m_ttVertsBuf->load(m_ttVerts, sizeof(SolidShaderVert) * 16);
}

void Tooltip::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);
    float pf = rootView().viewRes().pixelFactor();
    setVerts(m_nomWidth, m_nomHeight, pf);
    m_ttBlock.setViewRect(root, sub);
    m_ttBlockBuf->load(&m_ttBlock, sizeof(ViewBlock));

    boo::SWindowRect textRect = sub;
    textRect.location[0] += TOOLTIP_MARGIN * pf;
    textRect.location[1] += TOOLTIP_MARGIN * 1.5 * pf;
    m_message->resized(root, textRect);

    textRect.location[1] += m_message->nominalHeight() + TOOLTIP_MARGIN * pf;
    m_title->resized(root, textRect);

    boo::SWindowRect cornerRect = sub;
    cornerRect.location[1] += m_nomHeight - TOOLTIP_MARGIN * pf;
    m_cornersOutline[0]->resized(root, cornerRect);
    m_cornersFilled[0]->resized(root, cornerRect);
    cornerRect.location[0] += m_nomWidth - TOOLTIP_MARGIN * pf;
    m_cornersOutline[1]->resized(root, cornerRect);
    m_cornersFilled[1]->resized(root, cornerRect);
    cornerRect.location[1] = sub.location[1];
    m_cornersOutline[2]->resized(root, cornerRect);
    m_cornersFilled[2]->resized(root, cornerRect);
    cornerRect.location[0] = sub.location[0];
    m_cornersOutline[3]->resized(root, cornerRect);
    m_cornersFilled[3]->resized(root, cornerRect);
}

void Tooltip::resetResources(ViewResources& res)
{
    for (int i=0 ; i<4 ; ++i)
    {
        m_cornersOutline[i].reset(new TextView(res, *this, res.m_curveFont, 1));
        m_cornersFilled[i].reset(new TextView(res, *this, res.m_curveFont, 1));
    }
    m_cornersOutline[0]->typesetGlyphs(L"\xF4F0");
    m_cornersFilled[0]->typesetGlyphs(L"\xF4F1", res.themeData().tooltipBackground());
    m_cornersOutline[1]->typesetGlyphs(L"\xF4F2");
    m_cornersFilled[1]->typesetGlyphs(L"\xF4F3", res.themeData().tooltipBackground());
    m_cornersOutline[2]->typesetGlyphs(L"\xF4F4");
    m_cornersFilled[2]->typesetGlyphs(L"\xF4F5", res.themeData().tooltipBackground());
    m_cornersOutline[3]->typesetGlyphs(L"\xF4F6");
    m_cornersFilled[3]->typesetGlyphs(L"\xF4F7", res.themeData().tooltipBackground());

    m_title.reset(new TextView(res, *this, res.m_heading14));
    m_title->typesetGlyphs(m_titleStr);
    m_message.reset(new MultiLineTextView(res, *this, res.m_mainFont));
    m_message->typesetGlyphs(m_messageStr, Zeus::CColor::skWhite,
                             int(TOOLTIP_MAX_TEXT_WIDTH * rootView().viewRes().pixelFactor()));

    float pf = res.pixelFactor();
    m_nomWidth = std::min(int(TOOLTIP_MAX_WIDTH * pf),
                          int(std::max(m_title->nominalWidth(), m_message->nominalWidth()) + TOOLTIP_MARGIN * 2 * pf));
    m_nomHeight = m_title->nominalHeight() + m_message->nominalHeight() + TOOLTIP_MARGIN * 3 * pf;
}

void Tooltip::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    gfxQ->setShaderDataBinding(m_ttShaderBinding);
    gfxQ->setDrawPrimitive(boo::Primitive::TriStrips);
    gfxQ->draw(0, 16);

    for (int i=0 ; i<4 ; ++i)
        m_cornersFilled[i]->draw(gfxQ);

    m_title->draw(gfxQ);
    m_message->draw(gfxQ);
}

}
