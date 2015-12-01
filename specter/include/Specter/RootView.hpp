#ifndef SPECTER_ROOTVIEW_HPP
#define SPECTER_ROOTVIEW_HPP

#include "View.hpp"
#include "MultiLineTextView.hpp"
#include "FontCache.hpp"
#include "DeferredWindowEvents.hpp"
#include <boo/boo.hpp>

namespace Specter
{
class ViewSystem;

class RootView : public View
{
    boo::IWindow* m_window = nullptr;
    boo::ITextureR* m_renderTex = nullptr;
    boo::SWindowRect m_rootRect = {};
    bool m_resizeRTDirty = false;
    bool m_destroyed = false;

    DeferredWindowEvents<RootView> m_events;

public:
    RootView(ViewSystem& system, boo::IWindow* window);

    void destroyed();
    bool isDestroyed() const {return m_destroyed;}

    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
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

    void dispatchEvents() {m_events.dispatchEvents();}
    void draw(boo::IGraphicsCommandQueue* gfxQ);
    const boo::SWindowRect& rootRect() const {return m_rootRect;}

    boo::IWindow* window() const {return m_window;}

    class SplitView : public View
    {
    public:
        class System
        {
            friend class ViewSystem;
            friend class SplitView;
            boo::ITextureS* m_shadingTex;

            void init(boo::IGraphicsDataFactory* factory);
        };

        enum class Axis
        {
            Horizontal,
            Vertical
        };
    private:
        Axis m_axis;
        float m_slide = 0.5;
        bool m_dragging = false;

        void setSlide(float slide)
        {
            m_slide = std::min(std::max(slide, 0.0f), 1.0f);
            updateSize();
        }

        std::unique_ptr<View> m_views[2];
        VertexBlock m_splitBlock;
        boo::IGraphicsBufferD* m_splitBlockBuf;
        struct SplitVert
        {
            Zeus::CVector3f m_pos;
            Zeus::CVector2f m_uv;
        } m_splitVerts[4];

        void setHorizontalVerts(int width)
        {
            m_splitVerts[0].m_pos.assign(0, 1, 0);
            m_splitVerts[0].m_uv.assign(0, 0);
            m_splitVerts[1].m_pos.assign(0, -1, 0);
            m_splitVerts[1].m_uv.assign(1, 0);
            m_splitVerts[2].m_pos.assign(width, 1, 0);
            m_splitVerts[2].m_uv.assign(0, 0);
            m_splitVerts[3].m_pos.assign(width, -1, 0);
            m_splitVerts[3].m_uv.assign(1, 0);
        }

        void setVerticalVerts(int height)
        {
            m_splitVerts[0].m_pos.assign(-1, height, 0);
            m_splitVerts[0].m_uv.assign(0, 0);
            m_splitVerts[1].m_pos.assign(-1, 0, 0);
            m_splitVerts[1].m_uv.assign(0, 0);
            m_splitVerts[2].m_pos.assign(1, height, 0);
            m_splitVerts[2].m_uv.assign(1, 0);
            m_splitVerts[3].m_pos.assign(1, 0, 0);
            m_splitVerts[3].m_uv.assign(1, 0);
        }

        boo::IGraphicsBufferD* m_splitVertsBuf;
        boo::IVertexFormat* m_splitVtxFmt; /* OpenGL only */
        boo::IShaderDataBinding* m_splitShaderBinding;
        int m_splitValidSlots = 0;
    public:
        SplitView(ViewSystem& system, View& parentView, Axis axis);
        void setContentView(int slot, std::unique_ptr<View>&& view);
        std::unique_ptr<View> releaseContentView(int slot);
        void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseMove(const boo::SWindowCoord&);
        void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
        void draw(boo::IGraphicsCommandQueue* gfxQ);
    };

private:
    std::unique_ptr<SplitView> m_splitView;
};

}

#endif // SPECTER_ROOTVIEW_HPP
