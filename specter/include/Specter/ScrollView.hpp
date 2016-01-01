#ifndef SPECTER_SCROLLVIEW_HPP
#define SPECTER_SCROLLVIEW_HPP

#include "View.hpp"

namespace Specter
{
class ViewResources;

class ScrollView : public View
{
public:
    enum class Style
    {
        Plain,
        ThinIndicator
    };

private:
    Style m_style;
    View* m_contentView = nullptr;
    int m_scroll[2] = {};
    int m_targetScroll[2] = {};

    size_t m_consecutiveIdx = 0;
    double m_consecutiveScroll[16][2] = {};

    bool m_drawInd = false;

    SolidShaderVert m_verts[4];
    boo::IGraphicsBufferD* m_vertsBuf = nullptr;
    boo::IVertexFormat* m_vtxFmt = nullptr; /* OpenGL only */
    boo::IShaderDataBinding* m_shaderBinding = nullptr;

    bool _scroll(const boo::SScrollDelta& scroll);
public:
    ScrollView(ViewResources& res, View& parentView, Style style);
    void setContentView(View* v)
    {
        m_contentView = v;
        updateSize();
    }

    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseMove(const boo::SWindowCoord&);
    void mouseEnter(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);
    void scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll);
    int getScrollX() const {return m_scroll[0];}
    int getScrollY() const {return m_scroll[1];}

    int nominalWidth() const {return subRect().size[0];}
    int nominalHeight() const {return subRect().size[1];}

    void setMultiplyColor(const Zeus::CColor& color)
    {
        View::setMultiplyColor(color);
        if (m_contentView)
            m_contentView->setMultiplyColor(color);
    }

    void think();
    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}

#endif // SPECTER_SCROLLVIEW_HPP
