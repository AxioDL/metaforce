#ifndef SPECTER_VIEW_HPP
#define SPECTER_VIEW_HPP

#include <boo/boo.hpp>
#include "optional.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CTransform.hpp"
#include "zeus/CColor.hpp"
#include "hecl/CVar.hpp"
#include "hecl/UniformBufferPool.hpp"
#include "hecl/VertexBufferPool.hpp"

#include <boo/graphicsdev/GL.hpp>
#include <boo/graphicsdev/D3D.hpp>
#include <boo/graphicsdev/Metal.hpp>
#include <boo/graphicsdev/Vulkan.hpp>

namespace specter
{
class IThemeData;
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
        zeus::CVector3f m_pos;
        zeus::CColor m_color = zeus::CColor::skClear;
    };
    struct TexShaderVert
    {
        zeus::CVector3f m_pos;
        zeus::CVector2f m_uv;
    };
    struct ViewBlock
    {
        zeus::CMatrix4f m_mv;
        zeus::CColor m_color = zeus::CColor::skWhite;
        void setViewRect(const boo::SWindowRect& root, const boo::SWindowRect& sub)
        {
            m_mv[0][0] = 2.0f / root.size[0];
            m_mv[1][1] = 2.0f / root.size[1];
            m_mv[3][0] = sub.location[0] * m_mv[0][0] - 1.0f;
            m_mv[3][1] = sub.location[1] * m_mv[1][1] - 1.0f;
        }
    };

    template <typename VertStruct>
    struct VertexBufferBinding
    {
        typename hecl::VertexBufferPool<VertStruct>::Token m_vertsBuf;
        boo::ObjToken<boo::IVertexFormat> m_vtxFmt; /* OpenGL only */
        boo::ObjToken<boo::IShaderDataBinding> m_shaderBinding;

        void load(const VertStruct* data, size_t count)
        {
            if (m_vertsBuf)
            {
                VertStruct* out = m_vertsBuf.access();
                for (size_t i=0; i<count; ++i)
                    out[i] = data[i];
            }
        }
        template <typename VertArray>
        void load(const VertArray data)
        {
            static_assert(std::is_same<std::remove_all_extents_t<VertArray>,
                          VertStruct>::value, "mismatched type");
            if (m_vertsBuf)
            {
                constexpr size_t count = sizeof(VertArray) / sizeof(VertStruct);
                VertStruct* out = m_vertsBuf.access();
                for (size_t i=0; i<count; ++i)
                    out[i] = data[i];
            }
        }

        operator const boo::ObjToken<boo::IShaderDataBinding>&() { return m_shaderBinding; }
    };
    struct VertexBufferBindingSolid : VertexBufferBinding<SolidShaderVert>
    {
        void init(boo::IGraphicsDataFactory::Context& ctx,
                  ViewResources& res, size_t count,
                  const hecl::UniformBufferPool<ViewBlock>::Token& viewBlockBuf);
    };
    struct VertexBufferBindingTex : VertexBufferBinding<TexShaderVert>
    {
        void init(boo::IGraphicsDataFactory::Context& ctx,
                  ViewResources& res, size_t count,
                  const hecl::UniformBufferPool<ViewBlock>::Token& viewBlockBuf,
                  const boo::ObjToken<boo::ITexture>& texture);
    };

private:
    RootView& m_rootView;
    View& m_parentView;
    boo::SWindowRect m_subRect;
    VertexBufferBindingSolid m_bgVertsBinding;
    SolidShaderVert m_bgRect[4];

    friend class RootView;
    View(ViewResources& res);

protected:
    ViewBlock m_viewVertBlock;
#define SPECTER_GLSL_VIEW_VERT_BLOCK\
    "UBINDING0 uniform SpecterViewBlock\n"\
    "{\n"\
    "    mat4 mv;\n"\
    "    vec4 mulColor;\n"\
    "};\n"
#define SPECTER_HLSL_VIEW_VERT_BLOCK\
    "cbuffer SpecterViewBlock : register(b0)\n"\
    "{\n"\
    "    float4x4 mv;\n"\
    "    float4 mulColor;\n"\
    "};\n"
#define SPECTER_METAL_VIEW_VERT_BLOCK\
    "struct SpecterViewBlock\n"\
    "{\n"\
    "    float4x4 mv;\n"\
    "    float4 mulColor;\n"\
    "};\n"
    hecl::UniformBufferPool<ViewBlock>::Token m_viewVertBlockBuf;

public:
    struct Resources
    {
        hecl::UniformBufferPool<ViewBlock> m_bufPool;
        hecl::VertexBufferPool<SolidShaderVert> m_solidPool;
        hecl::VertexBufferPool<TexShaderVert> m_texPool;

        void updateBuffers()
        {
            m_bufPool.updateBuffers();
            m_solidPool.updateBuffers();
            m_texPool.updateBuffers();
        }

        boo::ObjToken<boo::IShaderPipeline> m_solidShader;
        boo::ObjToken<boo::IVertexFormat> m_solidVtxFmt; /* Not OpenGL */

        boo::ObjToken<boo::IShaderPipeline> m_texShader;
        boo::ObjToken<boo::IVertexFormat> m_texVtxFmt; /* Not OpenGL */

#if BOO_HAS_GL
        void init(boo::GLDataFactory::Context& ctx, const IThemeData& theme);
#endif
#if _WIN32
        void init(boo::D3DDataFactory::Context& ctx, const IThemeData& theme);
#endif
#if BOO_HAS_METAL
        void init(boo::MetalDataFactory::Context& ctx, const IThemeData& theme);
#endif
#if BOO_HAS_VULKAN
        void init(boo::VulkanDataFactory::Context& ctx, const IThemeData& theme);
#endif

        void destroy()
        {
            m_bufPool.doDestroy();
            m_solidPool.doDestroy();
            m_texPool.doDestroy();

            m_solidShader.reset();
            m_solidVtxFmt.reset();
            m_texShader.reset();
            m_texVtxFmt.reset();
        }
    };

protected:
    View(ViewResources& res, View& parentView);
    void buildResources(boo::IGraphicsDataFactory::Context& ctx, ViewResources& res);
    void commitResources(ViewResources& res, const boo::FactoryCommitFunc& commitFunc);

public:
    virtual ~View() {}
    View() = delete;
    View(const View& other) = delete;
    View& operator=(const View& other) = delete;

    View& parentView() {return m_parentView;}
    RootView& rootView() {return m_rootView;}
    const RootView& rootView() const {return m_rootView;}
    const boo::SWindowRect& subRect() const {return m_subRect;}
    int width() const {return m_subRect.size[0];}
    int height() const {return m_subRect.size[1];}
    void updateSize();

    void setBackground(const zeus::CColor& color)
    {
        for (int i=0 ; i<4 ; ++i)
            m_bgRect[i].m_color = color;
        m_bgVertsBinding.load<decltype(m_bgRect)>(m_bgRect);
    }

    virtual void setMultiplyColor(const zeus::CColor& color)
    {
        m_viewVertBlock.m_color = color;
        if (m_viewVertBlockBuf)
            m_viewVertBlockBuf.access() = m_viewVertBlock;
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

    virtual void containerResized(const boo::SWindowRect& root, const boo::SWindowRect& sub) {}
    virtual void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    virtual void resized(const ViewBlock& vb, const boo::SWindowRect& sub);
    virtual void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub,
                         const boo::SWindowRect& scissor) {resized(root, sub);}
    virtual void think() {}
    virtual void draw(boo::IGraphicsCommandQueue* gfxQ);
};

template <class ViewPtrType>
struct ViewChild
{
    ViewPtrType m_view = ViewPtrType();
    bool m_mouseIn = false;
    int m_mouseDown = 0;

    bool mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
    {
        if (!m_view)
            return false;
        if (m_view->subRect().coordInRect(coord))
        {
            if ((m_mouseDown & 1 << int(button)) == 0)
            {
                m_view->mouseDown(coord, button, mod);
                m_mouseDown |= 1 << int(button);
            }
            return true;
        }
        return false;
    }

    void mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
    {
        if (!m_view)
            return;
        if ((m_mouseDown & 1 << int(button)) != 0)
        {
            m_view->mouseUp(coord, button, mod);
            m_mouseDown &= ~(1 << int(button));
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

    void scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll)
    {
        if (!m_view)
            return;
        if (m_mouseIn)
            m_view->scroll(coord, scroll);
    }
};

template <class ViewPtrType>
struct ScissorViewChild : ViewChild<ViewPtrType>
{
    using base = ViewChild<ViewPtrType>;
    boo::SWindowRect m_scissorRect;

    bool mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
    {
        if (!base::m_view)
            return false;
        if (base::m_view->subRect().coordInRect(coord) &&
            m_scissorRect.coordInRect(coord))
        {
            if ((base::m_mouseDown & 1 << int(button)) == 0)
            {
                base::m_view->mouseDown(coord, button, mod);
                base::m_mouseDown |= 1 << int(button);
            }
            return true;
        }
        return false;
    }

    void mouseMove(const boo::SWindowCoord& coord)
    {
        if (!base::m_view)
            return;
        if (base::m_view->subRect().coordInRect(coord) &&
            m_scissorRect.coordInRect(coord))
        {
            if (!base::m_mouseIn)
            {
                base::m_view->mouseEnter(coord);
                base::m_mouseIn = true;
            }
            base::m_view->mouseMove(coord);
        }
        else
        {
            if (base::m_mouseIn)
            {
                base::m_view->mouseLeave(coord);
                base::m_mouseIn = false;
            }
        }
    }
};

}

#endif // SPECTER_VIEW_HPP
