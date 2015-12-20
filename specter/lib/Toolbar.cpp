#include <LogVisor/LogVisor.hpp>
#include "Specter/Toolbar.hpp"
#include "Specter/ViewResources.hpp"

#define TOOLBAR_GAUGE 28
#define TOOLBAR_PADDING 10

namespace Specter
{
static LogVisor::LogModule Log("Specter::Space");

void Toolbar::Resources::init(boo::IGraphicsDataFactory* factory, const ThemeData& theme)
{
    static const Zeus::RGBA32 tex[] =
    {
        {255,255,255,64},
        {255,255,255,64},
        {0,0,0,64},
        {0,0,0,64}
    };
    m_shadingTex = factory->newStaticTexture(4, 1, 1, boo::TextureFormat::RGBA8, tex, 16);
}

Toolbar::Toolbar(ViewResources& res, View& parentView, Position tbPos)
: View(res, parentView), m_tbPos(tbPos),
  m_nomHeight(res.pixelFactor() * TOOLBAR_GAUGE),
  m_padding(res.pixelFactor() * TOOLBAR_PADDING)
{
    m_tbBlockBuf = res.m_factory->newDynamicBuffer(boo::BufferUse::Uniform, sizeof(ViewBlock), 1);
    m_tbVertsBuf = res.m_factory->newDynamicBuffer(boo::BufferUse::Vertex, sizeof(TexShaderVert), 10);

    if (!res.m_viewRes.m_texVtxFmt)
    {
        boo::VertexElementDescriptor vdescs[] =
        {
            {m_tbVertsBuf, nullptr, boo::VertexSemantic::Position4},
            {m_tbVertsBuf, nullptr, boo::VertexSemantic::UV4}
        };
        m_tbVtxFmt = res.m_factory->newVertexFormat(2, vdescs);
        boo::IGraphicsBuffer* bufs[] = {m_tbBlockBuf};
        boo::ITexture* texs[] = {res.m_toolbarRes.m_shadingTex};
        m_tbShaderBinding = res.m_factory->newShaderDataBinding(res.m_viewRes.m_texShader,
                                                                m_tbVtxFmt, m_tbVertsBuf, nullptr,
                                                                nullptr, 1, bufs, 1, texs);
    }
    else
    {
        boo::IGraphicsBuffer* bufs[] = {m_tbBlockBuf};
        boo::ITexture* texs[] = {res.m_toolbarRes.m_shadingTex};
        m_tbShaderBinding = res.m_factory->newShaderDataBinding(res.m_viewRes.m_texShader,
                                                                res.m_viewRes.m_texVtxFmt,
                                                                m_tbVertsBuf, nullptr,
                                                                nullptr, 1, bufs, 1, texs);
    }

    setBackground(res.themeData().toolbarBackground());
    commitResources(res);
}

void Toolbar::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    for (ViewChild<View*>& c : m_children)
        c.mouseDown(coord, button, mod);
}

void Toolbar::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    for (ViewChild<View*>& c : m_children)
        c.mouseUp(coord, button, mod);
}

void Toolbar::mouseMove(const boo::SWindowCoord& coord)
{
    for (ViewChild<View*>& c : m_children)
        c.mouseMove(coord);
}

void Toolbar::mouseEnter(const boo::SWindowCoord& coord)
{
    for (ViewChild<View*>& c : m_children)
        c.mouseEnter(coord);
}

void Toolbar::mouseLeave(const boo::SWindowCoord& coord)
{
    for (ViewChild<View*>& c : m_children)
        c.mouseLeave(coord);
}

void Toolbar::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);
    setHorizontalVerts(sub.size[0]);
    m_tbVertsBuf->load(&m_tbVerts, sizeof(TexShaderVert) * 10);
    m_tbBlock.setViewRect(root, sub);
    m_tbBlockBuf->load(&m_tbBlock, sizeof(ViewBlock));

    boo::SWindowRect childRect = sub;
    for (ViewChild<View*>& c : m_children)
    {
        childRect.size[0] = c.m_view->nominalWidth();
        childRect.size[1] = c.m_view->nominalHeight();
        childRect.location[0] += m_padding;
        childRect.location[1] = sub.location[1] + (m_nomHeight - childRect.size[1]) / 2 - 1;
        c.m_view->resized(root, childRect);
        childRect.location[0] += childRect.size[0];
    }
}

void Toolbar::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);
    gfxQ->setShaderDataBinding(m_tbShaderBinding);
    gfxQ->setDrawPrimitive(boo::Primitive::TriStrips);
    gfxQ->draw(0, 10);

    for (ViewChild<View*>& c : m_children)
        c.m_view->draw(gfxQ);
}

}

