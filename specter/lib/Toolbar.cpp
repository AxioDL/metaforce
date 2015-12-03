#include <LogVisor/LogVisor.hpp>
#include "Specter/Toolbar.hpp"
#include "Specter/ViewResources.hpp"

namespace Specter
{
static LogVisor::LogModule Log("Specter::Space");

void Toolbar::Resources::init(boo::IGraphicsDataFactory* factory)
{
    static const Zeus::RGBA32 tex[3] =
    {
        {0,0,0,64},
        {0,0,0,0},
        {255,255,255,64}
    };
    m_shadingTex = factory->newStaticTexture(3, 1, 1, boo::TextureFormat::RGBA8, tex, 12);
}

Toolbar::Toolbar(ViewResources& system, View& parentView, Position tbPos)
: View(system, parentView), m_tbPos(tbPos), m_gauge(system.pixelFactor() * 25)
{
    m_tbBlockBuf = system.m_factory->newDynamicBuffer(boo::BufferUse::Uniform, sizeof(ViewBlock), 1);
    m_tbVertsBuf = system.m_factory->newDynamicBuffer(boo::BufferUse::Vertex, sizeof(ToolbarVert), 10);

    if (!system.m_viewRes.m_texVtxFmt)
    {
        boo::VertexElementDescriptor vdescs[] =
        {
            {m_tbVertsBuf, nullptr, boo::VertexSemantic::Position4},
            {m_tbVertsBuf, nullptr, boo::VertexSemantic::UV4}
        };
        m_tbVtxFmt = system.m_factory->newVertexFormat(2, vdescs);
        boo::IGraphicsBuffer* bufs[] = {m_tbBlockBuf};
        boo::ITexture* texs[] = {system.m_toolbarRes.m_shadingTex};
        m_tbShaderBinding = system.m_factory->newShaderDataBinding(system.m_viewRes.m_texShader,
                                                                   m_tbVtxFmt, m_tbVertsBuf, nullptr,
                                                                   nullptr, 1, bufs, 1, texs);
    }
    else
    {
        boo::IGraphicsBuffer* bufs[] = {m_tbBlockBuf};
        boo::ITexture* texs[] = {system.m_toolbarRes.m_shadingTex};
        m_tbShaderBinding = system.m_factory->newShaderDataBinding(system.m_viewRes.m_texShader,
                                                                   system.m_viewRes.m_texVtxFmt,
                                                                   m_tbVertsBuf, nullptr,
                                                                   nullptr, 1, bufs, 1, texs);
    }

    commitResources(system);
}

void Toolbar::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
}

void Toolbar::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
}

void Toolbar::mouseMove(const boo::SWindowCoord& coord)
{
}

void Toolbar::resetResources(ViewResources& res)
{
    m_gauge = res.pixelFactor() * 25;
    updateSize();
}

void Toolbar::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);
    setHorizontalVerts(sub.size[0]);
    m_tbVertsBuf->load(&m_tbVerts, sizeof(ToolbarVert) * 10);
}

void Toolbar::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);

}

}

