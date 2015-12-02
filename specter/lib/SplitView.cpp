#include <LogVisor/LogVisor.hpp>
#include "Specter/SplitView.hpp"
#include "Specter/ViewResources.hpp"

namespace Specter
{
static LogVisor::LogModule Log("Specter::SplitView");

void SplitView::Resources::init(boo::IGraphicsDataFactory* factory)
{
    static const Zeus::RGBA32 tex[3] =
    {
        {0,0,0,64},
        {0,0,0,255},
        {255,255,255,64}
    };
    m_shadingTex = factory->newStaticTexture(3, 1, 1, boo::TextureFormat::RGBA8, tex, 12);
}

SplitView::SplitView(ViewResources& system, View& parentView, Axis axis)
: View(system, parentView), m_axis(axis)
{
    m_splitBlockBuf = system.m_factory->newDynamicBuffer(boo::BufferUse::Uniform, sizeof(VertexBlock), 1);
    m_splitVertsBuf = system.m_factory->newDynamicBuffer(boo::BufferUse::Vertex, sizeof(SplitVert), 4);

    if (!system.m_viewRes.m_texVtxFmt)
    {
        boo::VertexElementDescriptor vdescs[] =
        {
            {m_splitVertsBuf, nullptr, boo::VertexSemantic::Position4},
            {m_splitVertsBuf, nullptr, boo::VertexSemantic::UV4}
        };
        m_splitVtxFmt = system.m_factory->newVertexFormat(2, vdescs);
        boo::IGraphicsBuffer* bufs[] = {m_splitBlockBuf};
        boo::ITexture* texs[] = {system.m_splitViewRes.m_shadingTex};
        m_splitShaderBinding = system.m_factory->newShaderDataBinding(system.m_viewRes.m_texShader,
                                                                 m_splitVtxFmt, m_splitVertsBuf, nullptr,
                                                                 nullptr, 1, bufs, 1, texs);
    }
    else
    {
        boo::IGraphicsBuffer* bufs[] = {m_splitBlockBuf};
        boo::ITexture* texs[] = {system.m_splitViewRes.m_shadingTex};
        m_splitShaderBinding = system.m_factory->newShaderDataBinding(system.m_viewRes.m_texShader,
                                                                 system.m_viewRes.m_texVtxFmt,
                                                                 m_splitVertsBuf, nullptr,
                                                                 nullptr, 1, bufs, 1, texs);
    }

    commitResources(system);
}

std::unique_ptr<View> SplitView::setContentView(int slot, std::unique_ptr<View>&& view)
{
    if (slot < 0 || slot > 1)
        Log.report(LogVisor::FatalError, "out-of-range slot to RootView::SplitView::setContentView");
    std::unique_ptr<View> ret;
    m_views[slot].swap(ret);
    m_views[slot] = std::move(view);
    updateSize();
    return ret;
}

void SplitView::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (button == boo::EMouseButton::Primary)
    {
        m_dragging = true;
        if (m_axis == Axis::Horizontal)
            setSlide(coord.pixel[1] / float(subRect().size[1]));
        else if (m_axis == Axis::Vertical)
            setSlide(coord.pixel[0] / float(subRect().size[0]));
    }
}

void SplitView::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (button == boo::EMouseButton::Primary)
        m_dragging = false;
}

void SplitView::mouseMove(const boo::SWindowCoord& coord)
{
    if (m_axis == Axis::Horizontal)
    {
        if (m_dragging)
            setSlide(coord.pixel[1] / float(subRect().size[1]));
        int slidePx = subRect().size[1] * m_slide;
        if (abs(int(coord.pixel[1]) - slidePx) < 4)
            root().window()->setCursor(boo::EMouseCursor::VerticalArrow);
        else
            root().window()->setCursor(boo::EMouseCursor::Pointer);
    }
    else if (m_axis == Axis::Vertical)
    {
        if (m_dragging)
            setSlide(coord.pixel[0] / float(subRect().size[0]));
        int slidePx = subRect().size[0] * m_slide;
        if (abs(int(coord.pixel[0]) - slidePx) < 4)
            root().window()->setCursor(boo::EMouseCursor::HorizontalArrow);
        else
            root().window()->setCursor(boo::EMouseCursor::Pointer);
    }
}

void SplitView::resetResources(ViewResources& res)
{
    if (m_views[0])
        m_views[0]->resetResources(res);
    if (m_views[1])
        m_views[1]->resetResources(res);
}

void SplitView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);
    if (m_axis == Axis::Horizontal)
    {
        boo::SWindowRect ssub = sub;
        ssub.size[1] *= m_slide;
        if (m_views[0])
            m_views[0]->resized(root, ssub);
        ssub.location[1] += ssub.size[1];
        ssub.size[1] = sub.size[1] - ssub.size[1];
        if (m_views[1])
            m_views[1]->resized(root, ssub);
        m_splitBlock.setViewRect(root, ssub);
        setHorizontalVerts(ssub.size[0]);
    }
    else if (m_axis == Axis::Vertical)
    {
        boo::SWindowRect ssub = sub;
        ssub.size[0] *= m_slide;
        if (m_views[0])
            m_views[0]->resized(root, ssub);
        ssub.location[0] += ssub.size[0];
        ssub.size[0] = sub.size[0] - ssub.size[0];
        if (m_views[1])
            m_views[1]->resized(root, ssub);
        m_splitBlock.setViewRect(root, ssub);
        setVerticalVerts(ssub.size[1]);
    }
    m_splitValid = false;
}

void SplitView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);
    if (m_views[0])
        m_views[0]->draw(gfxQ);
    if (m_views[1])
        m_views[1]->draw(gfxQ);

    if (!m_splitValid)
    {
        m_splitBlockBuf->load(&m_splitBlock, sizeof(VertexBlock));
        m_splitVertsBuf->load(m_splitVerts, sizeof(SplitVert) * 4);
        m_splitValid = true;
    }
    gfxQ->setShaderDataBinding(m_splitShaderBinding);
    gfxQ->draw(0, 4);

}

}

