#include "Specter/RootView.hpp"
#include "Specter/ViewSystem.hpp"

namespace Specter
{
static LogVisor::LogModule Log("Specter::RootView");

RootView::RootView(ViewSystem& system, boo::IWindow* window)
: View(system, *this), m_window(window)
{
    window->setCallback(this);
    boo::SWindowRect rect = window->getWindowFrame();
    m_renderTex = system.m_factory->newRenderTexture(rect.size[0], rect.size[1], 1);
    commitResources(system);
    m_splitView.reset(new SplitView(system, *this, SplitView::Axis::Horizontal));
    MultiLineTextView* textView1 = new MultiLineTextView(system, *this, system.m_mainFont);
    MultiLineTextView* textView2 = new MultiLineTextView(system, *this, system.m_mainFont);
    m_splitView->setContentView(0, std::unique_ptr<MultiLineTextView>(textView1));
    m_splitView->setContentView(1, std::unique_ptr<MultiLineTextView>(textView2));
    resized(rect);
    textView1->typesetGlyphs("Hello, World!\n\n", Zeus::CColor::skWhite);
    textView2->typesetGlyphs("こんにちは世界！\n\n", Zeus::CColor::skWhite);
    Zeus::CColor transBlack(0.f, 0.f, 0.f, 0.5f);
    textView1->setBackground(transBlack);
    textView2->setBackground(transBlack);
    setBackground(Zeus::CColor::skGrey);
}

void RootView::destroyed()
{
    m_destroyed = true;
}

void RootView::resized(const boo::SWindowRect& rect)
{
    resized(rect, rect);
}

void RootView::resized(const boo::SWindowRect& root, const boo::SWindowRect&)
{
    m_rootRect = root;
    m_rootRect.location[0] = 0;
    m_rootRect.location[1] = 0;
    View::resized(m_rootRect, m_rootRect);
    m_splitView->resized(m_rootRect, m_rootRect);
    m_resizeRTDirty = true;
}

void RootView::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods)
{
    m_splitView->mouseDown(coord, button, mods);
}

void RootView::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods)
{
    m_splitView->mouseUp(coord, button, mods);
}

void RootView::mouseMove(const boo::SWindowCoord& coord)
{
    m_splitView->mouseMove(coord);
}

void RootView::mouseEnter(const boo::SWindowCoord& coord)
{
}

void RootView::mouseLeave(const boo::SWindowCoord& coord)
{
}

void RootView::scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll)
{
}

void RootView::touchDown(const boo::STouchCoord& coord, uintptr_t tid)
{
}

void RootView::touchUp(const boo::STouchCoord& coord, uintptr_t tid)
{
}

void RootView::touchMove(const boo::STouchCoord& coord, uintptr_t tid)
{
}

void RootView::charKeyDown(unsigned long charCode, boo::EModifierKey mods, bool isRepeat)
{
}

void RootView::charKeyUp(unsigned long charCode, boo::EModifierKey mods)
{
}

void RootView::specialKeyDown(boo::ESpecialKey key, boo::EModifierKey mods, bool isRepeat)
{
}

void RootView::specialKeyUp(boo::ESpecialKey key, boo::EModifierKey mods)
{
}

void RootView::modKeyDown(boo::EModifierKey mod, bool isRepeat)
{
}

void RootView::modKeyUp(boo::EModifierKey mod)
{
}

void RootView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    if (m_resizeRTDirty)
    {
        gfxQ->resizeRenderTexture(m_renderTex, m_rootRect.size[0], m_rootRect.size[1]);
        m_resizeRTDirty = false;
    }
    gfxQ->setRenderTarget(m_renderTex);
    gfxQ->setViewport(m_rootRect);
    gfxQ->setScissor(m_rootRect);
    View::draw(gfxQ);
    m_splitView->draw(gfxQ);
    gfxQ->resolveDisplay(m_renderTex);
}

void RootView::SplitView::System::init(boo::IGraphicsDataFactory* factory)
{
    static const Zeus::RGBA32 tex[3] =
    {
        {0,0,0,64},
        {0,0,0,255},
        {255,255,255,64}
    };
    m_shadingTex = factory->newStaticTexture(3, 1, 1, boo::TextureFormat::RGBA8, tex, 12);
}

RootView::SplitView::SplitView(ViewSystem& system, View& parentView, Axis axis)
: View(system, parentView), m_axis(axis)
{
    m_splitBlockBuf = system.m_factory->newDynamicBuffer(boo::BufferUse::Uniform, sizeof(VertexBlock), 1);
    m_splitVertsBuf = system.m_factory->newDynamicBuffer(boo::BufferUse::Vertex, sizeof(SplitVert), 4);

    if (!system.m_viewSystem.m_texVtxFmt)
    {
        boo::VertexElementDescriptor vdescs[] =
        {
            {m_splitVertsBuf, nullptr, boo::VertexSemantic::Position4},
            {m_splitVertsBuf, nullptr, boo::VertexSemantic::UV4}
        };
        m_splitVtxFmt = system.m_factory->newVertexFormat(2, vdescs);
        boo::IGraphicsBuffer* bufs[] = {m_splitBlockBuf};
        boo::ITexture* texs[] = {system.m_splitViewSystem.m_shadingTex};
        m_splitShaderBinding = system.m_factory->newShaderDataBinding(system.m_viewSystem.m_texShader,
                                                                 m_splitVtxFmt, m_splitVertsBuf, nullptr,
                                                                 nullptr, 1, bufs, 1, texs);
    }
    else
    {
        boo::IGraphicsBuffer* bufs[] = {m_splitBlockBuf};
        boo::ITexture* texs[] = {system.m_splitViewSystem.m_shadingTex};
        m_splitShaderBinding = system.m_factory->newShaderDataBinding(system.m_viewSystem.m_texShader,
                                                                 system.m_viewSystem.m_texVtxFmt,
                                                                 m_splitVertsBuf, nullptr,
                                                                 nullptr, 1, bufs, 1, texs);
    }

    commitResources(system);
}

void RootView::SplitView::setContentView(int slot, std::unique_ptr<View>&& view)
{
    if (slot < 0 || slot > 1)
        Log.report(LogVisor::FatalError, "out-of-range slot to RootView::SplitView::setContentView");
    m_views[slot] = std::move(view);
    updateSize();
}

std::unique_ptr<View> RootView::SplitView::releaseContentView(int slot)
{
    if (slot < 0 || slot > 1)
        Log.report(LogVisor::FatalError, "out-of-range slot to RootView::SplitView::releaseContentView");
    std::unique_ptr<View> ret;
    m_views[slot].swap(ret);
    updateSize();
    return ret;
}

void RootView::SplitView::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
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

void RootView::SplitView::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (button == boo::EMouseButton::Primary)
        m_dragging = false;
}

void RootView::SplitView::mouseMove(const boo::SWindowCoord& coord)
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

void RootView::SplitView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
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
    m_splitValidSlots = 0;
}

void RootView::SplitView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);
    if (m_views[0])
        m_views[0]->draw(gfxQ);
    if (m_views[1])
        m_views[1]->draw(gfxQ);

    int pendingSlot = 1 << gfxQ->pendingDynamicSlot();
    if ((m_splitValidSlots & pendingSlot) == 0)
    {
        m_splitBlockBuf->load(&m_splitBlock, sizeof(VertexBlock));
        m_splitVertsBuf->load(m_splitVerts, sizeof(SplitVert) * 4);
        m_splitValidSlots |= pendingSlot;
    }
    gfxQ->setShaderDataBinding(m_splitShaderBinding);
    gfxQ->draw(0, 4);

}

}
