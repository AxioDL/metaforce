#ifndef SPECTER_ICON_HPP
#define SPECTER_ICON_HPP

#include "View.hpp"

namespace specter
{

struct Icon
{
    boo::ObjToken<boo::ITexture> m_tex;
    zeus::CVector2f m_uvCoords[4];
    Icon() = default;
    Icon(const boo::ObjToken<boo::ITexture>& tex, float rect[4])
    : m_tex(tex)
    {
        m_uvCoords[0][0] = rect[0];
        m_uvCoords[0][1] = -rect[3];

        m_uvCoords[1][0] = rect[0];
        m_uvCoords[1][1] = -rect[1];

        m_uvCoords[2][0] = rect[2];
        m_uvCoords[2][1] = -rect[3];

        m_uvCoords[3][0] = rect[2];
        m_uvCoords[3][1] = -rect[1];
    }
};

template <size_t COLS, size_t ROWS>
class IconAtlas
{
    boo::ObjToken<boo::ITextureS> m_tex;
    Icon m_icons[COLS][ROWS];

    Icon MakeIcon(float x, float y)
    {
        float rect[] = {x / float(COLS),                     y / float(ROWS),
                        x / float(COLS) + 1.f / float(COLS), y / float(ROWS) + 1.f / float(ROWS)};
        return Icon(m_tex.get(), rect);
    }
public:
    IconAtlas() = default;
    operator bool() const {return m_tex;}
    void initializeAtlas(const boo::ObjToken<boo::ITextureS>& tex)
    {
        m_tex = tex;
        for (int c=0 ; c<COLS ; ++c)
            for (int r=0 ; r<ROWS ; ++r)
                m_icons[c][r] = MakeIcon(c, r);
    }
    void destroyAtlas() { m_tex.reset(); }
    Icon& getIcon(size_t c, size_t r) {return m_icons[c][r];}
};

class IconView : public View
{
    VertexBufferBindingTex m_vertexBinding;
public:
    IconView(ViewResources& res, View& parentView, Icon& icon);
    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}

#endif // SPECTER_ICON_HPP
