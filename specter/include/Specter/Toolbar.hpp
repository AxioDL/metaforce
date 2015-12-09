#ifndef SPECTER_TOOLBAR_HPP
#define SPECTER_TOOLBAR_HPP

#include "Specter/View.hpp"

namespace Specter
{

class Toolbar : public View
{
public:
    class Resources
    {
        friend class ViewResources;
        friend class Toolbar;
        boo::ITextureS* m_shadingTex;

        void init(boo::IGraphicsDataFactory* factory, const ThemeData& theme);
    };

    enum class Position
    {
        Bottom,
        Top
    };
private:
    Position m_tbPos;
    struct Child
    {
        std::unique_ptr<View> m_view;
        bool m_mouseIn = false;
        bool m_mouseDown = false;
        Child(std::unique_ptr<View>&& v) : m_view(std::move(v)) {}
    };
    std::vector<Child> m_children;

    ViewBlock m_tbBlock;
    boo::IGraphicsBufferD* m_tbBlockBuf;
    TexShaderVert m_tbVerts[10];
    int m_nomHeight = 25;
    int m_padding = 10;

    void setHorizontalVerts(int width)
    {
        m_tbVerts[0].m_pos.assign(0, 1 + m_nomHeight, 0);
        m_tbVerts[0].m_uv.assign(0, 0);
        m_tbVerts[1].m_pos.assign(0, -1 + m_nomHeight, 0);
        m_tbVerts[1].m_uv.assign(0.5, 0);
        m_tbVerts[2].m_pos.assign(width, 1 + m_nomHeight, 0);
        m_tbVerts[2].m_uv.assign(0, 0);
        m_tbVerts[3].m_pos.assign(width, -1 + m_nomHeight, 0);
        m_tbVerts[3].m_uv.assign(0.5, 0);
        m_tbVerts[4].m_pos.assign(width, -1 + m_nomHeight, 0);
        m_tbVerts[4].m_uv.assign(0.5, 0);

        m_tbVerts[5].m_pos.assign(0, 1, 0);
        m_tbVerts[5].m_uv.assign(0.5, 0);
        m_tbVerts[6].m_pos.assign(0, 1, 0);
        m_tbVerts[6].m_uv.assign(0.5, 0);
        m_tbVerts[7].m_pos.assign(0, -1, 0);
        m_tbVerts[7].m_uv.assign(1, 0);
        m_tbVerts[8].m_pos.assign(width, 1, 0);
        m_tbVerts[8].m_uv.assign(0.5, 0);
        m_tbVerts[9].m_pos.assign(width, -1, 0);
        m_tbVerts[9].m_uv.assign(1, 0);
    }

    void setVerticalVerts(int height)
    {
        m_tbVerts[0].m_pos.assign(-1, height, 0);
        m_tbVerts[0].m_uv.assign(0, 0);
        m_tbVerts[1].m_pos.assign(-1, 0, 0);
        m_tbVerts[1].m_uv.assign(0, 0);
        m_tbVerts[2].m_pos.assign(1, height, 0);
        m_tbVerts[2].m_uv.assign(0.5, 0);
        m_tbVerts[3].m_pos.assign(1, 0, 0);
        m_tbVerts[3].m_uv.assign(0.5, 0);
        m_tbVerts[4].m_pos.assign(1, 0, 0);
        m_tbVerts[4].m_uv.assign(0.5, 0);

        m_tbVerts[5].m_pos.assign(-1 + m_nomHeight, height, 0);
        m_tbVerts[5].m_uv.assign(0.5, 0);
        m_tbVerts[6].m_pos.assign(-1 + m_nomHeight, height, 0);
        m_tbVerts[6].m_uv.assign(0.5, 0);
        m_tbVerts[7].m_pos.assign(-1 + m_nomHeight, 0, 0);
        m_tbVerts[7].m_uv.assign(0.5, 0);
        m_tbVerts[8].m_pos.assign(1 + m_nomHeight, height, 0);
        m_tbVerts[8].m_uv.assign(1, 0);
        m_tbVerts[9].m_pos.assign(1 + m_nomHeight, 0, 0);
        m_tbVerts[9].m_uv.assign(1, 0);
    }

    boo::IGraphicsBufferD* m_tbVertsBuf;
    boo::IVertexFormat* m_tbVtxFmt; /* OpenGL only */
    boo::IShaderDataBinding* m_tbShaderBinding;
public:
    Toolbar(ViewResources& res, View& parentView, Position toolbarPos);
    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseMove(const boo::SWindowCoord&);
    void mouseEnter(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&coord);
    void resized(const boo::SWindowRect& rootView, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);

    int nominalHeight() const {return m_nomHeight;}

    void clear() {m_children.clear();}
    void push_back(std::unique_ptr<View>&& v) {m_children.push_back(std::move(v));}
};

}

#endif // SPECTER_TOOLBAR_HPP
