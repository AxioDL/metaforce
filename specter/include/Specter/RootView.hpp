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
#include "IViewManager.hpp"
#include <boo/boo.hpp>

namespace Specter
{

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
    Button* m_activeMenuButton = nullptr;

    ViewChild<std::unique_ptr<View>> m_rightClickMenu;
    boo::SWindowCoord m_rightClickMenuCoord;

    SplitView* m_hoverSplitDragView = nullptr;
    bool m_activeSplitDragView = false;
    SplitView* recursiveTestSplitHover(SplitView* sv, const boo::SWindowCoord& coord) const;

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
    const IThemeData& themeData() const {return *m_viewRes->m_theme;}

    std::vector<View*>& accessContentViews() {return m_views;}

    void adoptRightClickMenu(std::unique_ptr<View>&& menu, const boo::SWindowCoord& coord)
    {
        m_rightClickMenu.m_view = std::move(menu);
        m_rightClickMenuCoord = coord;
    }
    View* getRightClickMenu() {return m_rightClickMenu.m_view.get();}

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
    void setActiveMenuButton(Button* button)
    {
        m_activeMenuButton = button;
    }
    void unsetActiveMenuButton(Button* button)
    {
        if (button == m_activeMenuButton)
            m_activeMenuButton = nullptr;
    }

    void startSplitDrag(SplitView* sv, const boo::SWindowCoord& coord)
    {
        m_hoverSplitDragView = sv;
        m_activeSplitDragView = true;
        sv->startDragSplit(coord);
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
    bool m_spaceCornerHover = false;
    void setSpaceCornerHover(bool hover)
    {
        m_spaceCornerHover = hover;
        _updateCursor();
    }

    void resetTooltip(ViewResources& res);
    void displayTooltip(const std::string& name, const std::string& help);

private:
    void _updateCursor()
    {
        if (m_spaceCornerHover)
            m_window->setCursor(boo::EMouseCursor::Crosshairs);
        else if (m_vSplitHover)
            m_window->setCursor(boo::EMouseCursor::HorizontalArrow);
        else if (m_hSplitHover)
            m_window->setCursor(boo::EMouseCursor::VerticalArrow);
        else if (m_textFieldHover)
            m_window->setCursor(boo::EMouseCursor::IBeam);
        else
            m_window->setCursor(boo::EMouseCursor::Pointer);
    }

    std::vector<View*> m_views;
    std::unique_ptr<Tooltip> m_tooltip;
};

}

#endif // SPECTER_ROOTVIEW_HPP
