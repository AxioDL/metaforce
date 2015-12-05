#include "Specter/RootView.hpp"
#include "Specter/ViewResources.hpp"
#include "Specter/Space.hpp"

namespace Specter
{
static LogVisor::LogModule Log("Specter::RootView");

RootView::RootView(ViewResources& res, boo::IWindow* window)
: View(res), m_window(window), m_events(*this), m_viewRes(&res)
{
    window->setCallback(&m_events);
    boo::SWindowRect rect = window->getWindowFrame();
    m_renderTex = res.m_factory->newRenderTexture(rect.size[0], rect.size[1], 1);
    commitResources(res);
    m_splitView.reset(new SplitView(res, *this, SplitView::Axis::Horizontal));
    Space* space1 = new Space(res, *m_splitView, Toolbar::Position::Bottom);
    space1->toolbar().push_back(std::make_unique<Button>(res, space1->toolbar(), "Hello Button"));
    MultiLineTextView* textView1 = new MultiLineTextView(res, *this, res.m_heading18);
    space1->setContentView(std::unique_ptr<MultiLineTextView>(textView1));
    Space* space2 = new Space(res, *m_splitView, Toolbar::Position::Bottom);
    space2->toolbar().push_back(std::make_unique<Button>(res, space2->toolbar(), "こんにちはボタン"));
    MultiLineTextView* textView2 = new MultiLineTextView(res, *this, res.m_heading18);
    space2->setContentView(std::unique_ptr<MultiLineTextView>(textView2));
    m_splitView->setContentView(0, std::unique_ptr<Space>(space1));
    m_splitView->setContentView(1, std::unique_ptr<Space>(space2));
    resized(rect, rect);
    textView1->typesetGlyphs("Hello, World!\n\n", res.themeData().uiText());
    textView2->typesetGlyphs("こんにちは世界！\n\n", res.themeData().uiText());
    textView1->setBackground(res.themeData().viewportBackground());
    textView2->setBackground(res.themeData().viewportBackground());
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
    m_splitView->mouseEnter(coord);
}

void RootView::mouseLeave(const boo::SWindowCoord& coord)
{
    m_splitView->mouseLeave(coord);
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
    m_splitView->resetResources(res);
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
