#include <LogVisor/LogVisor.hpp>
#include "Specter/Toolbar.hpp"
#include "Specter/ViewResources.hpp"

#define TOOLBAR_PADDING 10

namespace Specter
{
static LogVisor::LogModule Log("Specter::Space");

void Toolbar::Resources::init(boo::IGraphicsDataFactory* factory, const IThemeData& theme)
{
    static const Zeus::RGBA32 tex[] =
    {
        {{255,255,255,64}},
        {{255,255,255,64}},
        {{0,0,0,64}},
        {{0,0,0,64}}
    };
    m_shadingTex = factory->newStaticTexture(4, 1, 1, boo::TextureFormat::RGBA8, tex, 16);
}

Toolbar::Toolbar(ViewResources& res, View& parentView, Position tbPos)
: View(res, parentView), m_tbPos(tbPos),
  m_nomHeight(res.pixelFactor() * SPECTER_TOOLBAR_GAUGE),
  m_padding(res.pixelFactor() * TOOLBAR_PADDING)
{
    m_tbBlockBuf = res.m_factory->newDynamicBuffer(boo::BufferUse::Uniform, sizeof(ViewBlock), 1);
    m_vertsBinding.initTex(res, 10, m_tbBlockBuf, res.m_toolbarRes.m_shadingTex);
    commitResources(res);
    setBackground(res.themeData().toolbarBackground());
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
    m_vertsBinding.load(m_tbVerts, sizeof(m_tbVerts));
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
    gfxQ->setShaderDataBinding(m_vertsBinding);
    gfxQ->setDrawPrimitive(boo::Primitive::TriStrips);
    gfxQ->draw(0, 10);

    for (ViewChild<View*>& c : m_children)
        c.m_view->draw(gfxQ);
}

}

