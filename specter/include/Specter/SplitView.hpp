#ifndef SPECTER_SPLITVIEW_HPP
#define SPECTER_SPLITVIEW_HPP

#include "Specter/View.hpp"

namespace Specter
{

class SplitView : public View
{
public:
    class Resources
    {
        friend class ViewResources;
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
    SplitView(ViewResources& res, View& parentView, Axis axis);
    std::unique_ptr<View> setContentView(int slot, std::unique_ptr<View>&& view);
    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseMove(const boo::SWindowCoord&);
    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}

#endif // SPECTER_SPLITVIEW_HPP
