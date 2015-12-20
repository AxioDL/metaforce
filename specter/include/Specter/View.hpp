#ifndef SPECTER_VIEW_HPP
#define SPECTER_VIEW_HPP

#include <boo/boo.hpp>
#include "CVector3f.hpp"
#include "CMatrix4f.hpp"
#include "CTransform.hpp"
#include "CColor.hpp"
#include "HECL/CVar.hpp"

#include <boo/graphicsdev/GL.hpp>
#include <boo/graphicsdev/D3D.hpp>
#include <boo/graphicsdev/Metal.hpp>

namespace Specter
{
class ThemeData;
class ViewResources;
class RootView;

class RectangleConstraint
{
public:
    enum class Test
    {
        Fixed,
        Minimum,
        Maximum
    };
private:
    int m_x, m_y;
    Test m_xtest, m_ytest;
public:
    RectangleConstraint(int x=-1, int y=-1, Test xtest=Test::Fixed, Test ytest=Test::Fixed)
    : m_x(x), m_y(y), m_xtest(xtest), m_ytest(ytest) {}
    std::pair<int,int> solve(int x, int y) const
    {
        std::pair<int,int> ret;

        if (m_x < 0)
            ret.first = x;
        else
        {
            switch (m_xtest)
            {
            case Test::Fixed:
                ret.first = m_x;
                break;
            case Test::Minimum:
                ret.first = std::max(m_x, x);
                break;
            case Test::Maximum:
                ret.first = std::min(m_x, x);
                break;
            }
        }

        if (m_y < 0)
            ret.second = y;
        else
        {
            switch (m_ytest)
            {
            case Test::Fixed:
                ret.second = m_y;
                break;
            case Test::Minimum:
                ret.second = std::max(m_y, y);
                break;
            case Test::Maximum:
                ret.second = std::min(m_y, y);
                break;
            }
        }

        return ret;
    }
};

class View
{
public:
    struct SolidShaderVert
    {
        Zeus::CVector3f m_pos;
        Zeus::CColor m_color = Zeus::CColor::skClear;
    };
    struct TexShaderVert
    {
        Zeus::CVector3f m_pos;
        Zeus::CVector2f m_uv;
    };
private:
    RootView& m_rootView;
    View& m_parentView;
    boo::SWindowRect m_subRect;
    boo::IGraphicsBufferD* m_bgVertBuf;
    boo::IVertexFormat* m_bgVtxFmt = nullptr; /* OpenGL only */
    boo::IShaderDataBinding* m_bgShaderBinding;
    SolidShaderVert m_bgRect[4];
    boo::GraphicsDataToken m_gfxData;

    friend class RootView;
    void buildResources(ViewResources& res);
    View(ViewResources& res);

protected:
    struct ViewBlock
    {
        Zeus::CMatrix4f m_mv;
        Zeus::CColor m_color = Zeus::CColor::skWhite;
        void setViewRect(const boo::SWindowRect& root, const boo::SWindowRect& sub)
        {
            m_mv[0][0] = 2.0f / root.size[0];
            m_mv[1][1] = 2.0f / root.size[1];
            m_mv[3][0] = sub.location[0] * m_mv[0][0] - 1.0f;
            m_mv[3][1] = sub.location[1] * m_mv[1][1] - 1.0f;
        }
    } m_viewVertBlock;
#define SPECTER_VIEW_VERT_BLOCK_GLSL\
    "uniform SpecterViewBlock\n"\
    "{\n"\
    "    mat4 mv;\n"\
    "    vec4 mulColor;\n"\
    "};\n"
#define SPECTER_VIEW_VERT_BLOCK_HLSL\
    "cbuffer SpecterViewBlock : register(b0)\n"\
    "{\n"\
    "    float4x4 mv;\n"\
    "    float4 mulColor;\n"\
    "};\n"
#define SPECTER_VIEW_VERT_BLOCK_METAL\
    "struct SpecterViewBlock\n"\
    "{\n"\
    "    float4x4 mv;\n"\
    "    float4 mulColor;\n"\
    "};\n"
    boo::IGraphicsBufferD* m_viewVertBlockBuf;

public:
    struct Resources
    {
        boo::IShaderPipeline* m_solidShader = nullptr;
        boo::IVertexFormat* m_solidVtxFmt = nullptr; /* Not OpenGL */

        boo::IShaderPipeline* m_texShader = nullptr;
        boo::IVertexFormat* m_texVtxFmt = nullptr; /* Not OpenGL */

        void init(boo::GLDataFactory* factory, const ThemeData& theme);
#if _WIN32
        void init(boo::ID3DDataFactory* factory, const ThemeData& theme);
#elif BOO_HAS_METAL
        void init(boo::MetalDataFactory* factory, const ThemeData& theme);
#endif
    };

protected:
    View(ViewResources& res, View& parentView);
    void commitResources(ViewResources& res);

public:
    virtual ~View() {}
    View() = delete;
    View(const View& other) = delete;
    View& operator=(const View& other) = delete;

    View& parentView() {return m_parentView;}
    RootView& rootView() {return m_rootView;}
    const boo::SWindowRect& subRect() const {return m_subRect;}
    int width() const {return m_subRect.size[0];}
    int height() const {return m_subRect.size[1];}
    void updateSize();

    void setBackground(const Zeus::CColor& color)
    {
        for (int i=0 ; i<4 ; ++i)
            m_bgRect[i].m_color = color;
        m_bgVertBuf->load(&m_bgRect, sizeof(SolidShaderVert) * 4);
    }
    void setMultiplyColor(const Zeus::CColor& color)
    {
        m_viewVertBlock.m_color = color;
        m_viewVertBlockBuf->load(&m_viewVertBlock, sizeof(ViewBlock));
    }

    virtual int nominalWidth() const {return 0;}
    virtual int nominalHeight() const {return 0;}

    virtual void setActive(bool) {}

    virtual void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) {}
    virtual void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) {}
    virtual void mouseMove(const boo::SWindowCoord&) {}
    virtual void mouseEnter(const boo::SWindowCoord&) {}
    virtual void mouseLeave(const boo::SWindowCoord&) {}
    virtual void scroll(const boo::SWindowCoord&, const boo::SScrollDelta&) {}
    virtual void touchDown(const boo::STouchCoord&, uintptr_t) {}
    virtual void touchUp(const boo::STouchCoord&, uintptr_t) {}
    virtual void touchMove(const boo::STouchCoord&, uintptr_t) {}
    virtual void charKeyDown(unsigned long, boo::EModifierKey, bool) {}
    virtual void charKeyUp(unsigned long, boo::EModifierKey) {}
    virtual void specialKeyDown(boo::ESpecialKey, boo::EModifierKey, bool) {}
    virtual void specialKeyUp(boo::ESpecialKey, boo::EModifierKey) {}
    virtual void modKeyDown(boo::EModifierKey, bool) {}
    virtual void modKeyUp(boo::EModifierKey) {}

    virtual void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    virtual void think() {}
    virtual void draw(boo::IGraphicsCommandQueue* gfxQ);
};

template <class ViewType>
struct ViewChild
{
    std::unique_ptr<ViewType> m_view;
    bool m_mouseIn = false;
    bool m_mouseDown = false;

    void mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
    {
        if (!m_view)
            return;
        if (m_view->subRect().coordInRect(coord))
        {
            if (!m_mouseDown)
            {
                m_view->mouseDown(coord, button, mod);
                m_mouseDown = true;
            }
        }
    }

    void mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
    {
        if (!m_view)
            return;
        if (m_mouseDown)
        {
            m_view->mouseUp(coord, button, mod);
            m_mouseDown = false;
        }
    }

    void mouseMove(const boo::SWindowCoord& coord)
    {
        if (!m_view)
            return;
        if (m_view->subRect().coordInRect(coord))
        {
            if (!m_mouseIn)
            {
                m_view->mouseEnter(coord);
                m_mouseIn = true;
            }
            m_view->mouseMove(coord);
        }
        else
        {
            if (m_mouseIn)
            {
                m_view->mouseLeave(coord);
                m_mouseIn = false;
            }
        }
    }

    void mouseEnter(const boo::SWindowCoord& coord)
    {
        if (!m_view)
            return;
    }

    void mouseLeave(const boo::SWindowCoord& coord)
    {
        if (!m_view)
            return;
        if (m_mouseIn)
        {
            m_view->mouseLeave(coord);
            m_mouseIn = false;
        }
    }
};

}

#endif // SPECTER_VIEW_HPP
