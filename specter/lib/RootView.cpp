#include "Specter/RootView.hpp"
#include "Specter/ViewResources.hpp"
#include "Specter/Space.hpp"

namespace Specter
{
static LogVisor::LogModule Log("Specter::RootView");

RootView::RootView(IViewManager& viewMan, ViewResources& res, boo::IWindow* window)
: View(res), m_window(window), m_viewMan(viewMan), m_viewRes(&res), m_events(*this)
{
    window->setCallback(&m_events);
    boo::SWindowRect rect = window->getWindowFrame();
    m_renderTex = res.m_factory->newRenderTexture(rect.size[0], rect.size[1], 1);
    commitResources(res);
    resized(rect, rect);
}

void RootView::destroyed()
{
    m_destroyed = true;
}

void RootView::resized(const boo::SWindowRect& root, const boo::SWindowRect&)
{
    m_rootRect = root;
    m_rootRect.location[0] = 0;
    m_rootRect.location[1] = 0;
    View::resized(m_rootRect, m_rootRect);
    if (m_view)
        m_view->resized(m_rootRect, m_rootRect);
    if (m_tooltip)
        m_tooltip->resized(m_rootRect, m_rootRect);
    m_resizeRTDirty = true;
}

void RootView::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods)
{
    if (m_activeTextView && !m_activeTextView->subRect().coordInRect(coord))
        setActiveTextView(nullptr);
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
    if (m_activeDragView)
        m_activeDragView->mouseMove(coord);

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
    if (m_view)
        m_view->scroll(coord, scroll);
}

void RootView::touchDown(const boo::STouchCoord& coord, uintptr_t tid)
{
    if (m_view)
        m_view->touchDown(coord, tid);
}

void RootView::touchUp(const boo::STouchCoord& coord, uintptr_t tid)
{
    if (m_view)
        m_view->touchUp(coord, tid);
}

void RootView::touchMove(const boo::STouchCoord& coord, uintptr_t tid)
{
    if (m_view)
        m_view->touchMove(coord, tid);
}

void RootView::charKeyDown(unsigned long charCode, boo::EModifierKey mods, bool isRepeat)
{
    if (m_view)
        m_view->charKeyDown(charCode, mods, isRepeat);
    if (m_activeTextView &&
        (mods & (boo::EModifierKey::Ctrl|boo::EModifierKey::Command)) != boo::EModifierKey::None)
    {
        if (charCode == 'c' || charCode == 'C')
            m_activeTextView->clipboardCopy();
        else if (charCode == 'x' || charCode == 'X')
            m_activeTextView->clipboardCut();
        else if (charCode == 'v' || charCode == 'V')
            m_activeTextView->clipboardPaste();
    }
}

void RootView::charKeyUp(unsigned long charCode, boo::EModifierKey mods)
{
    if (m_view)
        m_view->charKeyUp(charCode, mods);
}

void RootView::specialKeyDown(boo::ESpecialKey key, boo::EModifierKey mods, bool isRepeat)
{
    if (key == boo::ESpecialKey::Enter && (mods & boo::EModifierKey::Alt) != boo::EModifierKey::None)
    {
        m_window->setFullscreen(!m_window->isFullscreen());
        return;
    }
    if (m_view)
        m_view->specialKeyDown(key, mods, isRepeat);
    if (m_activeTextView)
        m_activeTextView->specialKeyDown(key, mods, isRepeat);
}

void RootView::specialKeyUp(boo::ESpecialKey key, boo::EModifierKey mods)
{
    if (m_view)
        m_view->specialKeyUp(key, mods);
    if (m_activeTextView)
        m_activeTextView->specialKeyUp(key, mods);
}

void RootView::modKeyDown(boo::EModifierKey mod, bool isRepeat)
{
    if (m_view)
        m_view->modKeyDown(mod, isRepeat);
    if (m_activeTextView)
        m_activeTextView->modKeyDown(mod, isRepeat);
}

void RootView::modKeyUp(boo::EModifierKey mod)
{
    if (m_view)
        m_view->modKeyUp(mod);
    if (m_activeTextView)
        m_activeTextView->modKeyUp(mod);
}

View* RootView::setContentView(View* view)
{
    View* ret = m_view;
    m_view = view;
    updateSize();
    return ret;
}

void RootView::resetTooltip(ViewResources& res)
{
    m_tooltip.reset(new Tooltip(res, *this, "Test", "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi hendrerit nisl quis lobortis mattis. Mauris efficitur, est a vestibulum iaculis, leo orci pellentesque nunc, non rutrum ipsum lectus eget nisl. Aliquam accumsan vestibulum turpis. Duis id lacus ac lectus sollicitudin posuere vel sit amet metus. Aenean nec tortor id enim efficitur accumsan vitae eu ante. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Fusce magna eros, lacinia a leo eget, volutpat rhoncus urna."));
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
        gfxQ->schedulePostFrameHandler([&](){m_events.m_resizeCv.notify_one();});
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
