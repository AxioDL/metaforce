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

    std::unique_ptr<View> m_contentView;
    ViewBlock m_tbBlock;
    boo::IGraphicsBufferD* m_tbBlockBuf;
    struct ToolbarVert
    {
        Zeus::CVector3f m_pos;
        Zeus::CVector2f m_uv;
    } m_tbVerts[10];
    int m_gauge = 25;

    void setHorizontalVerts(int width)
    {
        m_tbVerts[0].m_pos.assign(0, 1 + m_gauge, 0);
        m_tbVerts[0].m_uv.assign(0, 0);
        m_tbVerts[1].m_pos.assign(0, -1 + m_gauge, 0);
        m_tbVerts[1].m_uv.assign(0.5, 0);
        m_tbVerts[2].m_pos.assign(width, 1 + m_gauge, 0);
        m_tbVerts[2].m_uv.assign(0, 0);
        m_tbVerts[3].m_pos.assign(width, -1 + m_gauge, 0);
        m_tbVerts[3].m_uv.assign(0.5, 0);
        m_tbVerts[4].m_pos.assign(width, -1 + m_gauge, 0);
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

        m_tbVerts[5].m_pos.assign(-1 + m_gauge, height, 0);
        m_tbVerts[5].m_uv.assign(0.5, 0);
        m_tbVerts[6].m_pos.assign(-1 + m_gauge, height, 0);
        m_tbVerts[6].m_uv.assign(0.5, 0);
        m_tbVerts[7].m_pos.assign(-1 + m_gauge, 0, 0);
        m_tbVerts[7].m_uv.assign(0.5, 0);
        m_tbVerts[8].m_pos.assign(1 + m_gauge, height, 0);
        m_tbVerts[8].m_uv.assign(1, 0);
        m_tbVerts[9].m_pos.assign(1 + m_gauge, 0, 0);
        m_tbVerts[9].m_uv.assign(1, 0);
    }

    boo::IGraphicsBufferD* m_tbVertsBuf;
    boo::IVertexFormat* m_tbVtxFmt; /* OpenGL only */
    boo::IShaderDataBinding* m_tbShaderBinding;
    bool m_splitValid = false;
public:
    Toolbar(ViewResources& res, View& parentView, Position toolbarPos);
    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseMove(const boo::SWindowCoord&);
    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void resetResources(ViewResources& res);
    void draw(boo::IGraphicsCommandQueue* gfxQ);

    int gauge() const {return m_gauge;}
};

}

#endif // SPECTER_TOOLBAR_HPP
