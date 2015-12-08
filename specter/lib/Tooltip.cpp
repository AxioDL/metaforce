#include "Specter/Tooltip.hpp"
#include "Specter/ViewResources.hpp"
#include "Specter/RootView.hpp"

namespace Specter
{

#define TOOLTIP_MAX_WIDTH 316
#define TOOLTIP_MAX_TEXT_WIDTH 300

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

void Tooltip::setVerts(int width, int height, float pf)
{
    std::pair<int,int> margin = m_cornersFilled[0]->queryGlyphDimensions(0);
    width = std::max(width, margin.first*2);
    height = std::max(height, margin.second*2);

    m_ttVerts[0].m_pos.assign(0, height-margin.second, 0);
    m_ttVerts[1].m_pos.assign(0, margin.second, 0);
    m_ttVerts[2].m_pos.assign(width, height-margin.second, 0);
    m_ttVerts[3].m_pos.assign(width, margin.second, 0);
    m_ttVerts[4].m_pos.assign(width, margin.second, 0);

    m_ttVerts[5].m_pos.assign(margin.first, height, 0);
    m_ttVerts[6].m_pos.assign(margin.first, height, 0);
    m_ttVerts[7].m_pos.assign(margin.first, height-margin.second, 0);
    m_ttVerts[8].m_pos.assign(width-margin.first, height, 0);
    m_ttVerts[9].m_pos.assign(width-margin.first, height-margin.second, 0);
    m_ttVerts[10].m_pos.assign(width-margin.first, height-margin.second, 0);

    m_ttVerts[11].m_pos.assign(margin.first, margin.second, 0);
    m_ttVerts[12].m_pos.assign(margin.first, margin.second, 0);
    m_ttVerts[13].m_pos.assign(margin.first, 0, 0);
    m_ttVerts[14].m_pos.assign(width-margin.first, margin.second, 0);
    m_ttVerts[15].m_pos.assign(width-margin.first, 0, 0);

    m_ttVertsBuf->load(m_ttVerts, sizeof(SolidShaderVert) * 16);
}

void Tooltip::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);
    float pf = rootView().viewRes().pixelFactor();
    setVerts(m_nomWidth, m_nomHeight, pf);
    m_ttBlock.setViewRect(root, sub);
    m_ttBlockBuf->load(&m_ttBlock, sizeof(ViewBlock));

    std::pair<int,int> margin = m_cornersFilled[0]->queryGlyphDimensions(0);

    boo::SWindowRect textRect = sub;
    textRect.location[0] += margin.first;
    textRect.location[1] += margin.second * 1.5;
    m_message->resized(root, textRect);

    textRect.location[1] += m_message->nominalHeight() + margin.second;
    m_title->resized(root, textRect);

    boo::SWindowRect cornerRect = sub;
    cornerRect.location[1] += m_nomHeight - margin.second; // Upper left
    m_cornersOutline[0]->resized(root, cornerRect);
    m_cornersFilled[0]->resized(root, cornerRect);
    cornerRect.location[0] += m_nomWidth - margin.first; // Upper right
    m_cornersOutline[1]->resized(root, cornerRect);
    m_cornersFilled[1]->resized(root, cornerRect);
    cornerRect.location[1] = sub.location[1]; // Lower right
    m_cornersOutline[2]->resized(root, cornerRect);
    m_cornersFilled[2]->resized(root, cornerRect);
    cornerRect.location[0] = sub.location[0]; // Lower left
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
                             int(TOOLTIP_MAX_TEXT_WIDTH * res.pixelFactor()));

    float pf = res.pixelFactor();
    std::pair<int,int> margin = m_cornersOutline[0]->queryGlyphDimensions(0);
    m_nomWidth = std::min(int(TOOLTIP_MAX_WIDTH * pf),
                          int(std::max(m_title->nominalWidth(), m_message->nominalWidth()) + margin.first * 2));
    m_nomHeight = m_title->nominalHeight() + m_message->nominalHeight() + margin.second * 3;
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
