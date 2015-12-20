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

        void init(boo::IGraphicsDataFactory* factory, const ThemeData& theme);
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

    ViewChild<View*> m_views[2];
    ViewBlock m_splitBlock;
    boo::IGraphicsBufferD* m_splitBlockBuf;
    TexShaderVert m_splitVerts[4];

    void setHorizontalVerts(int width)
    {
        m_splitVerts[0].m_pos.assign(0, 2, 0);
        m_splitVerts[0].m_uv.assign(0, 0);
        m_splitVerts[1].m_pos.assign(0, -1, 0);
        m_splitVerts[1].m_uv.assign(1, 0);
        m_splitVerts[2].m_pos.assign(width, 2, 0);
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
        m_splitVerts[2].m_pos.assign(2, height, 0);
        m_splitVerts[2].m_uv.assign(1, 0);
        m_splitVerts[3].m_pos.assign(2, 0, 0);
        m_splitVerts[3].m_uv.assign(1, 0);
    }

    boo::IGraphicsBufferD* m_splitVertsBuf;
    boo::IVertexFormat* m_splitVtxFmt; /* OpenGL only */
    boo::IShaderDataBinding* m_splitShaderBinding;
public:
    SplitView(ViewResources& res, View& parentView, Axis axis);
    View* setContentView(int slot, View* view);
    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseMove(const boo::SWindowCoord&);
    void mouseEnter(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);
    void resized(const boo::SWindowRect& rootView, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}

#endif // SPECTER_SPLITVIEW_HPP
