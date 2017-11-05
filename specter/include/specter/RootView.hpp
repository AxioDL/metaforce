#ifndef SPECTER_ROOTVIEW_HPP
#define SPECTER_ROOTVIEW_HPP

#include "View.hpp"
#include "ViewResources.hpp"
#include "MultiLineTextView.hpp"
#include "TextField.hpp"
#include "SplitView.hpp"
#include "Tooltip.hpp"
#include "FontCache.hpp"
#include "IMenuNode.hpp"
#include "IViewManager.hpp"
#include "optional.hpp"
#include <boo/boo.hpp>

namespace specter
{

class RootView : public View
{
    boo::IWindow* m_window = nullptr;
    boo::ObjToken<boo::ITextureR> m_renderTex;
    boo::SWindowRect m_rootRect = {};
    bool m_resizeRTDirty = false;
    bool m_destroyed = false;
    IViewManager& m_viewMan;
    ViewResources* m_viewRes;
    ITextInputView* m_activeTextView = nullptr;
    View* m_activeDragView = nullptr;
    Button* m_activeMenuButton = nullptr;

    ViewChild<std::unique_ptr<View>> m_rightClickMenu;
    boo::SWindowRect m_rightClickMenuRootAndLoc;

    SplitView* m_hoverSplitDragView = nullptr;
    bool m_activeSplitDragView = false;
    SplitView* recursiveTestSplitHover(SplitView* sv, const boo::SWindowCoord& coord) const;

    boo::DeferredWindowEvents<RootView> m_events;

    struct SplitMenuSystem : IMenuNode
    {
        RootView& m_rv;
        std::string m_text;

        SplitView* m_splitView = nullptr;
        enum class Phase
        {
            Inactive,
            InteractiveSplit,
            InteractiveJoin,
        } m_phase = Phase::Inactive;
        int m_interactiveSlot = 0;
        float m_interactiveSplit = 0.5;
        bool m_interactiveDown = false;

        VertexBufferBindingSolid m_vertsBinding;
        ViewBlock m_viewBlock;
        hecl::UniformBufferPool<ViewBlock>::Token m_viewVertBlockBuf;
        SolidShaderVert m_verts[32];
        void setArrowVerts(const boo::SWindowRect& rect, SplitView::ArrowDir dir);
        void setLineVerts(const boo::SWindowRect& rect, float split, SplitView::Axis axis);

        void mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods);
        void mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods);
        void mouseMove(const boo::SWindowCoord& coord);
        void mouseLeave(const boo::SWindowCoord& coord);

        void resized();
        void draw(boo::IGraphicsCommandQueue* gfxQ);

        SplitMenuSystem(RootView& rv, boo::IGraphicsDataFactory::Context& ctx);
        const std::string* text() const {return &m_text;}
        size_t subNodeCount() const {return 2;}
        IMenuNode* subNode(size_t idx)
        {
            if (idx)
                return &m_joinActionNode;
            else
                return &m_splitActionNode;
        }

        boo::SWindowCoord m_deferredCoord;
        bool m_deferredSplit = false;
        bool m_deferredJoin = false;

        struct SplitActionNode : IMenuNode
        {
            SplitMenuSystem& m_smn;
            std::string m_text;
            SplitActionNode(SplitMenuSystem& smn);
            const std::string* text() const {return &m_text;}
            void activated(const boo::SWindowCoord& coord)
            {
                m_smn.m_deferredSplit = true;
                m_smn.m_deferredCoord = coord;
            }
        } m_splitActionNode;
        struct JoinActionNode : IMenuNode
        {
            SplitMenuSystem& m_smn;
            std::string m_text;
            JoinActionNode(SplitMenuSystem& smn);
            const std::string* text() const {return &m_text;}
            void activated(const boo::SWindowCoord& coord)
            {
                m_smn.m_deferredJoin = true;
                m_smn.m_deferredCoord = coord;
            }
        } m_joinActionNode;
    };
    std::experimental::optional<SplitMenuSystem> m_splitMenuSystem;

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

    void internalThink();
    void dispatchEvents() {m_events.dispatchEvents();}
    void draw(boo::IGraphicsCommandQueue* gfxQ);
    const boo::SWindowRect& rootRect() const {return m_rootRect;}

    boo::IWindow* window() const {return m_window;}
    IViewManager& viewManager() const {return m_viewMan;}
    ViewResources& viewRes() const {return *m_viewRes;}
    const IThemeData& themeData() const {return *m_viewRes->m_theme;}
    const boo::ObjToken<boo::ITextureR>& renderTex() const {return m_renderTex;}

    std::vector<View*>& accessContentViews() {return m_views;}

    void adoptRightClickMenu(std::unique_ptr<View>&& menu, const boo::SWindowCoord& coord)
    {
        m_rightClickMenu.m_view = std::move(menu);
        m_rightClickMenuRootAndLoc = subRect();
        m_rightClickMenuRootAndLoc.location[0] = coord.pixel[0];
        m_rightClickMenuRootAndLoc.location[1] = coord.pixel[1];
        updateSize();
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
    void unsetActiveDragView(View* dragView)
    {
        if (dragView == m_activeDragView)
            m_activeDragView = nullptr;
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

    void beginInteractiveJoin(SplitView* sv, const boo::SWindowCoord& coord)
    {
        m_splitMenuSystem->m_phase = SplitMenuSystem::Phase::InteractiveJoin;
        m_splitMenuSystem->m_splitView = sv;
        m_splitMenuSystem->mouseMove(coord);
    }

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
