#ifndef SPECTER_TOOLBAR_HPP
#define SPECTER_TOOLBAR_HPP

#include "specter/View.hpp"

namespace specter
{
#define SPECTER_TOOLBAR_GAUGE 28

class Toolbar : public View
{
public:
    class Resources
    {
        friend class ViewResources;
        friend class Toolbar;
        boo::ITextureS* m_shadingTex;

        void init(boo::IGraphicsDataFactory::Context& ctx, const IThemeData& theme);
    };

    enum class Position
    {
        None,
        Bottom,
        Top
    };
private:
    Position m_tbPos;
    unsigned m_units;
    std::vector<std::vector<ViewChild<View*>>> m_children;

    ViewBlock m_tbBlock;
    boo::IGraphicsBufferD* m_tbBlockBuf;
    TexShaderVert m_tbVerts[10];
    int m_nomGauge = 25;
    int m_padding = 10;

    void setHorizontalVerts(int width);
    void setVerticalVerts(int height);

    VertexBufferBinding m_vertsBinding;

public:
    Toolbar(ViewResources& res, View& parentView, Position toolbarPos, unsigned units);
    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseMove(const boo::SWindowCoord&);
    void mouseEnter(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&coord);
    void resized(const boo::SWindowRect& rootView, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);

    int nominalHeight() const
    {
        return m_nomGauge;
    }

    void clear()
    {
        for (std::vector<ViewChild<View*>>& u : m_children)
            u.clear();
    }
    void push_back(View* v, unsigned unit);

    void setMultiplyColor(const zeus::CColor& color)
    {
        View::setMultiplyColor(color);
        for (std::vector<ViewChild<View*>>& u : m_children)
            for (ViewChild<View*>& c : u)
                if (c.m_view)
                    c.m_view->setMultiplyColor(color);
    }
};

}

#endif // SPECTER_TOOLBAR_HPP
