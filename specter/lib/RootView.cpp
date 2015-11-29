#include "Specter/RootView.hpp"
#include "Specter/ViewSystem.hpp"

namespace Specter
{

RootView::RootView(ViewSystem& system, boo::IWindow* window)
: View(system, *this), m_window(window), m_textView(system, *this, system.m_mainFont)
{
    window->setCallback(this);
    boo::SWindowRect rect = window->getWindowFrame();
    m_renderTex = system.m_factory->newRenderTexture(rect.size[0], rect.size[1], 1);
    system.m_factory->commit();
    resized(rect);
    m_textView.typesetGlyphs("Hello, World!\nこんにちは世界！\n\n", Zeus::CColor::skWhite);
    Zeus::CColor transBlack(0.f, 0.f, 0.f, 0.5f);
    m_textView.setBackground(transBlack);
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

    boo::SWindowRect textRect = m_rootRect;
    textRect.location[0] = 10;
    textRect.location[1] = 10;
    textRect.size[0] -= 20;
    if (textRect.size[0] < 0)
        textRect.size[0] = 0;
    textRect.size[1] = 100;
    m_textView.resized(m_rootRect, textRect);

    m_resizeRTDirty = true;
}

void RootView::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods)
{
}

void RootView::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods)
{
}

void RootView::mouseMove(const boo::SWindowCoord& coord)
{
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
    m_textView.draw(gfxQ);
    gfxQ->resolveDisplay(m_renderTex);
}

}
