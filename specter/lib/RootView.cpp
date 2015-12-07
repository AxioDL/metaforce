#include "Specter/RootView.hpp"
#include "Specter/ViewResources.hpp"
#include "Specter/Space.hpp"

namespace Specter
{
static LogVisor::LogModule Log("Specter::RootView");

RootView::RootView(IViewManager& viewMan, ViewResources& res, boo::IWindow* window)
: View(res), m_window(window), m_events(*this), m_viewMan(viewMan), m_viewRes(&res)
{
    window->setCallback(&m_events);
    boo::SWindowRect rect = window->getWindowFrame();
    m_renderTex = res.m_factory->newRenderTexture(rect.size[0], rect.size[1], 1);
    commitResources(res);
    resized(rect, rect);

    m_tooltip.reset(new Tooltip(res, *this, "Test", "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi hendrerit nisl quis lobortis mattis. Mauris efficitur, est a vestibulum iaculis, leo orci pellentesque nunc, non rutrum ipsum lectus eget nisl. Aliquam accumsan vestibulum turpis. Duis id lacus ac lectus sollicitudin posuere vel sit amet metus. Aenean nec tortor id enim efficitur accumsan vitae eu ante. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Fusce magna eros, lacinia a leo eget, volutpat rhoncus urna."));
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
    if (m_view)
        m_view->resized(m_rootRect, m_rootRect);
    if (m_tooltip)
        m_tooltip->resized(m_rootRect, m_rootRect);
    if (old != m_rootRect)
        m_resizeRTDirty = true;
}

void RootView::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods)
{
    if (m_view)
        m_view->mouseDown(coord, button, mods);
}

void RootView::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods)
{
    if (m_view)
        m_view->mouseUp(coord, button, mods);
}

void RootView::mouseMove(const boo::SWindowCoord& coord)
{
    if (m_view)
        m_view->mouseMove(coord);

    boo::SWindowRect ttrect = m_rootRect;
    ttrect.location[0] = coord.pixel[0];
    ttrect.location[1] = coord.pixel[1];
    if (m_tooltip)
    {
        if (coord.pixel[0] + m_tooltip->nominalWidth() > m_rootRect.size[0])
            ttrect.location[0] -= m_tooltip->nominalWidth();
        if (coord.pixel[1] + m_tooltip->nominalHeight() > m_rootRect.size[1])
            ttrect.location[1] -= m_tooltip->nominalHeight();
        m_tooltip->resized(m_rootRect, ttrect);
    }
}

void RootView::mouseEnter(const boo::SWindowCoord& coord)
{
    if (m_view)
        m_view->mouseEnter(coord);
}

void RootView::mouseLeave(const boo::SWindowCoord& coord)
{
    if (m_view)
        m_view->mouseLeave(coord);
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
    if (key == boo::ESpecialKey::Enter && (mods & boo::EModifierKey::Alt) != boo::EModifierKey::None)
        m_window->setFullscreen(!m_window->isFullscreen());
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

void RootView::resetResources(ViewResources& res)
{
    m_viewRes = &res;
    if (m_view)
        m_view->resetResources(res);
    if (m_tooltip)
        m_tooltip->resetResources(res);
}

void RootView::setContentView(std::unique_ptr<View>&& view)
{
    m_view = std::move(view);
    updateSize();
}

void RootView::displayTooltip(const std::string& name, const std::string& help)
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
    if (m_view)
        m_view->draw(gfxQ);
    if (m_tooltip)
        m_tooltip->draw(gfxQ);
    gfxQ->resolveDisplay(m_renderTex);
}

}
