#ifndef SPECTER_ROOTVIEW_HPP
#define SPECTER_ROOTVIEW_HPP

#include "View.hpp"
#include "ViewResources.hpp"
#include "MultiLineTextView.hpp"
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

    void resetResources(ViewResources& res);
    void dispatchEvents() {m_events.dispatchEvents();}
    void draw(boo::IGraphicsCommandQueue* gfxQ);
    const boo::SWindowRect& rootRect() const {return m_rootRect;}

    boo::IWindow* window() const {return m_window;}
    IViewManager& viewManager() const {return m_viewMan;}
    const ViewResources& viewRes() const {return *m_viewRes;}
    const ThemeData& themeData() const {return m_viewRes->m_theme;}

    void setContentView(std::unique_ptr<View>&& view);

    void displayTooltip(const std::string& name, const std::string& help);

private:
    std::unique_ptr<View> m_view;
    std::unique_ptr<Tooltip> m_tooltip;
};

}

#endif // SPECTER_ROOTVIEW_HPP
