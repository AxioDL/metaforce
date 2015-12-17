#include "Specter/TextField.hpp"
#include "Specter/RootView.hpp"
#include "Specter/ViewResources.hpp"

namespace Specter
{

TextField::TextField(ViewResources& res, View& parentView)
: View(res, parentView)
{
    m_bVertsBuf = res.m_factory->newDynamicBuffer(boo::BufferUse::Vertex, sizeof(SolidShaderVert), 28);

    if (!res.m_viewRes.m_texVtxFmt)
    {
        boo::VertexElementDescriptor vdescs[] =
        {
            {m_bVertsBuf, nullptr, boo::VertexSemantic::Position4},
            {m_bVertsBuf, nullptr, boo::VertexSemantic::Color}
        };
        m_bVtxFmt = res.m_factory->newVertexFormat(2, vdescs);
        boo::IGraphicsBuffer* bufs[] = {m_viewVertBlockBuf};
        m_bShaderBinding = res.m_factory->newShaderDataBinding(res.m_viewRes.m_solidShader,
                                                               m_bVtxFmt, m_bVertsBuf, nullptr,
                                                               nullptr, 1, bufs, 0, nullptr);
    }
    else
    {
        boo::IGraphicsBuffer* bufs[] = {m_viewVertBlockBuf};
        m_bShaderBinding = res.m_factory->newShaderDataBinding(res.m_viewRes.m_solidShader,
                                                               res.m_viewRes.m_texVtxFmt,
                                                               m_bVertsBuf, nullptr,
                                                               nullptr, 1, bufs, 0, nullptr);
    }
    commitResources(res);

    m_verts[0].m_color = rootView().themeData().textfield1Inactive();
    m_verts[1].m_color = rootView().themeData().textfield2Inactive();
    m_verts[2].m_color = rootView().themeData().textfield1Inactive();
    m_verts[3].m_color = rootView().themeData().textfield2Inactive();
    m_verts[4].m_color = rootView().themeData().textfield2Inactive();
    for (int i=5 ; i<28 ; ++i)
        m_verts[i].m_color = res.themeData().textfield2Inactive();
    m_bVertsBuf->load(m_verts, sizeof(SolidShaderVert) * 28);

    m_text.reset(new TextView(res, *this, res.m_mainFont, TextView::Alignment::Left, 1024));
    setText("Test");
}

void TextField::setText(const std::string& str)
{
    m_textStr = str;
    m_text->typesetGlyphs(str, rootView().themeData().fieldText());
}

void TextField::setInactive()
{
    m_verts[0].m_color = rootView().themeData().textfield1Inactive();
    m_verts[1].m_color = rootView().themeData().textfield2Inactive();
    m_verts[2].m_color = rootView().themeData().textfield1Inactive();
    m_verts[3].m_color = rootView().themeData().textfield2Inactive();
    m_verts[4].m_color = rootView().themeData().textfield2Inactive();
    m_bVertsBuf->load(m_verts, sizeof(SolidShaderVert) * 28);
}

void TextField::setHover()
{
    m_verts[0].m_color = rootView().themeData().textfield1Hover();
    m_verts[1].m_color = rootView().themeData().textfield2Hover();
    m_verts[2].m_color = rootView().themeData().textfield1Hover();
    m_verts[3].m_color = rootView().themeData().textfield2Hover();
    m_verts[4].m_color = rootView().themeData().textfield2Hover();
    m_bVertsBuf->load(m_verts, sizeof(SolidShaderVert) * 28);
}

void TextField::setDisabled()
{
    m_verts[0].m_color = rootView().themeData().textfield1Disabled();
    m_verts[1].m_color = rootView().themeData().textfield2Disabled();
    m_verts[2].m_color = rootView().themeData().textfield1Disabled();
    m_verts[3].m_color = rootView().themeData().textfield2Disabled();
    m_verts[4].m_color = rootView().themeData().textfield2Disabled();
    m_bVertsBuf->load(m_verts, sizeof(SolidShaderVert) * 28);
}

void TextField::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
}

void TextField::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
}

void TextField::mouseMove(const boo::SWindowCoord& coord)
{
}

void TextField::mouseEnter(const boo::SWindowCoord& coord)
{
}

void TextField::mouseLeave(const boo::SWindowCoord& coord)
{
}

void TextField::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);

    float pf = rootView().viewRes().pixelFactor();
    int width = sub.size[0];
    int height = 20 * pf;

    m_verts[0].m_pos.assign(1, height+1, 0);
    m_verts[1].m_pos.assign(1, 1, 0);
    m_verts[2].m_pos.assign(width+1, height+1, 0);
    m_verts[3].m_pos.assign(width+1, 1, 0);
    m_verts[4].m_pos.assign(width+1, 1, 0);

    m_verts[5].m_pos.assign(1, height+1, 0);
    m_verts[6].m_pos.assign(1, height+1, 0);
    m_verts[7].m_pos.assign(0, height+1, 0);
    m_verts[8].m_pos.assign(1, 1, 0);
    m_verts[9].m_pos.assign(0, 1, 0);
    m_verts[10].m_pos.assign(0, 1, 0);

    m_verts[11].m_pos.assign(width+2, height+1, 0);
    m_verts[12].m_pos.assign(width+2, height+1, 0);
    m_verts[13].m_pos.assign(width+1, height+1, 0);
    m_verts[14].m_pos.assign(width+2, 1, 0);
    m_verts[15].m_pos.assign(width+1, 1, 0);
    m_verts[16].m_pos.assign(width+1, 1, 0);

    m_verts[17].m_pos.assign(1, height+2, 0);
    m_verts[18].m_pos.assign(1, height+2, 0);
    m_verts[19].m_pos.assign(1, height+1, 0);
    m_verts[20].m_pos.assign(width+1, height+2, 0);
    m_verts[21].m_pos.assign(width+1, height+1, 0);
    m_verts[22].m_pos.assign(width+1, height+1, 0);

    m_verts[23].m_pos.assign(1, 1, 0);
    m_verts[24].m_pos.assign(1, 1, 0);
    m_verts[25].m_pos.assign(1, 0, 0);
    m_verts[26].m_pos.assign(width+1, 1, 0);
    m_verts[27].m_pos.assign(width+1, 0, 0);

    m_bVertsBuf->load(m_verts, sizeof(SolidShaderVert) * 28);

    m_nomWidth = width;
    m_nomHeight = height;

    boo::SWindowRect textRect = sub;
    textRect.location[0] += 5 * pf;
    textRect.location[1] += 8 * pf;
    m_text->resized(root, textRect);
}

void TextField::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);
    gfxQ->setShaderDataBinding(m_bShaderBinding);
    gfxQ->setDrawPrimitive(boo::Primitive::TriStrips);
    gfxQ->draw(0, 28);
    m_text->draw(gfxQ);
}

}
