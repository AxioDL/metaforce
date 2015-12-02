#include "Specter/RootView.hpp"
#include "Specter/ViewResources.hpp"

namespace Specter
{
static LogVisor::LogModule Log("Specter::RootView");

RootView::RootView(ViewResources& res, boo::IWindow* window)
: View(res, *this), m_window(window), m_events(*this)
{
    window->setCallback(&m_events);
    boo::SWindowRect rect = window->getWindowFrame();
    m_renderTex = res.m_factory->newRenderTexture(rect.size[0], rect.size[1], 1);
    commitResources(res);
    m_splitView.reset(new SplitView(res, *this, SplitView::Axis::Horizontal));
    MultiLineTextView* textView1 = new MultiLineTextView(res, *this, res.m_heading18);
    MultiLineTextView* textView2 = new MultiLineTextView(res, *this, res.m_heading18);
    m_splitView->setContentView(0, std::unique_ptr<MultiLineTextView>(textView1));
    m_splitView->setContentView(1, std::unique_ptr<MultiLineTextView>(textView2));
    resized(rect, rect);
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

void RootView::resized(const boo::SWindowRect& root, const boo::SWindowRect&)
{
    boo::SWindowRect old = m_rootRect;
    m_rootRect = root;
    m_rootRect.location[0] = 0;
    m_rootRect.location[1] = 0;
    View::resized(m_rootRect, m_rootRect);
    m_splitView->resized(m_rootRect, m_rootRect);
    if (old != m_rootRect)
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

}
