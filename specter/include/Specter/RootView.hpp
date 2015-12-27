#ifndef SPECTER_ROOTVIEW_HPP
#define SPECTER_ROOTVIEW_HPP

#include "View.hpp"
#include "ViewResources.hpp"
#include "MultiLineTextView.hpp"
#include "TextField.hpp"
#include "SplitView.hpp"
#include "Tooltip.hpp"
#include "FontCache.hpp"
#include "DeferredWindowEvents.hpp"
#include <boo/boo.hpp>

namespace Specter
{

struct IViewManager
{
};

class RootView : public View
{
    boo::IWindow* m_window = nullptr;
    boo::ITextureR* m_renderTex = nullptr;
    boo::SWindowRect m_rootRect = {};
    bool m_resizeRTDirty = false;
    bool m_destroyed = false;
    IViewManager& m_viewMan;
    ViewResources* m_viewRes;
    ITextInputView* m_activeTextView = nullptr;
    View* m_activeDragView = nullptr;

    DeferredWindowEvents<RootView> m_events;

public:
    RootView(IViewManager& viewMan, ViewResources& res, boo::IWindow* window);

    void destroyed();
    bool isDestroyed() const {return m_destroyed;}

    void resized(const boo::SWindowRect& rootView, const boo::SWindowRect& sub);
    void mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods);
    void mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods);
    void mouseMove(const boo::SWindowCoord& coord);
    void mouseEnter(const boo::SWindowCoord& coord);
    void mouseLeave(const boo::SWindowCoord& coord);
    void scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll);

    void touchDown(const boo::STouchCoord& coord, uintptr_t tid);
    void touchUp(const boo::STouchCoord& coord, uintptr_t tid);
    void touchMove(const boo::STouchCoord& coord, uintptr_t tid);

    void charKeyDown(unsigned long charCode, boo::EModifierKey mods, bool isRepeat);
    void charKeyUp(unsigned long charCode, boo::EModifierKey mods);
    void specialKeyDown(boo::ESpecialKey key, boo::EModifierKey mods, bool isRepeat);
    void specialKeyUp(boo::ESpecialKey key, boo::EModifierKey mods);
    void modKeyDown(boo::EModifierKey mod, bool isRepeat);
    void modKeyUp(boo::EModifierKey mod);
    boo::ITextInputCallback* getTextInputCallback() {return m_activeTextView;}

    void dispatchEvents() {m_events.dispatchEvents();}
    void draw(boo::IGraphicsCommandQueue* gfxQ);
    const boo::SWindowRect& rootRect() const {return m_rootRect;}

    boo::IWindow* window() const {return m_window;}
    IViewManager& viewManager() const {return m_viewMan;}
    ViewResources& viewRes() const {return *m_viewRes;}
    const ThemeData& themeData() const {return m_viewRes->m_theme;}

    View* setContentView(View* view);
    void setActiveTextView(ITextInputView* textView)
    {
        if (m_activeTextView)
            m_activeTextView->setActive(false);
        m_activeTextView = textView;
        if (textView)
            textView->setActive(true);
    }
    void setActiveDragView(View* dragView)
    {
        m_activeDragView = dragView;
    }

    bool m_hSplitHover = false;
    void setHorizontalSplitHover(bool hover)
    {
        m_hSplitHover = hover;
        _updateCursor();
    }
    bool m_vSplitHover = false;
    void setVerticalSplitHover(bool hover)
    {
        m_vSplitHover = hover;
        _updateCursor();
    }
    bool m_textFieldHover = false;
    void setTextFieldHover(bool hover)
    {
        m_textFieldHover = hover;
        _updateCursor();
    }

    void resetTooltip(ViewResources& res);
    void displayTooltip(const std::string& name, const std::string& help);

private:
    void _updateCursor()
    {
        if (m_vSplitHover)
            m_window->setCursor(boo::EMouseCursor::HorizontalArrow);
        else if (m_hSplitHover)
            m_window->setCursor(boo::EMouseCursor::VerticalArrow);
        else if (m_textFieldHover)
            m_window->setCursor(boo::EMouseCursor::IBeam);
        else
            m_window->setCursor(boo::EMouseCursor::Pointer);
    }

    View* m_view = nullptr;
    std::unique_ptr<Tooltip> m_tooltip;
};

}

#endif // SPECTER_ROOTVIEW_HPP
