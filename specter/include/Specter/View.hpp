#ifndef SPECTER_VIEW_HPP
#define SPECTER_VIEW_HPP

#include <boo/boo.hpp>
#include "CVector3f.hpp"
#include "CMatrix4f.hpp"
#include "CTransform.hpp"
#include "CColor.hpp"

#include <boo/graphicsdev/GL.hpp>
#include <boo/graphicsdev/D3D.hpp>
#include <boo/graphicsdev/Metal.hpp>

namespace Specter
{
class ViewSystem;
class RootView;

class View
{
    RootView& m_rootView;
    View& m_parentView;
    boo::SWindowRect m_subRect;
    boo::IGraphicsBufferD* m_bgVertBuf;
    boo::IGraphicsBufferD* m_bgInstBuf;
    boo::IVertexFormat* m_bgVtxFmt = nullptr; /* OpenGL only */
    boo::IShaderDataBinding* m_bgShaderBinding;
    Zeus::CVector3f m_bgRect[4];
    Zeus::CColor m_bgColor;
    int m_bgValidSlots = 0;

    friend class RootView;
    void buildResources(ViewSystem& system);
    View(ViewSystem& system, RootView& parentView);

protected:
    struct VertexBlock
    {
        Zeus::CMatrix4f m_mv;
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
    "};\n"
#define SPECTER_VIEW_VERT_BLOCK_HLSL\
    "cbuffer SpecterViewBlock : register(b0)\n"\
    "{\n"\
    "    float4x4 mv;\n"\
    "};\n"
#define SPECTER_VIEW_VERT_BLOCK_METAL\
    "struct SpecterViewBlock\n"\
    "{\n"\
    "    float4x4 mv;\n"\
    "};\n"
    boo::IGraphicsBufferD* m_viewVertBlockBuf;

public:
    class System
    {
        friend class ViewSystem;
        friend class View;
        boo::IShaderPipeline* m_solidShader = nullptr;
        boo::IVertexFormat* m_vtxFmt = nullptr; /* Not OpenGL */

        void init(boo::GLDataFactory* factory);
#if _WIN32
        void init(boo::ID3DDataFactory* factory);
#elif BOO_HAS_METAL
        void init(boo::MetalDataFactory* factory);
#endif
    };

protected:
    View(ViewSystem& system, View& parentView);

public:
    View() = delete;

    View& parent() {return m_parentView;}
    RootView& root() {return m_rootView;}
    void updateSize();

    void setBackground(Zeus::CColor color) {m_bgColor = color; m_bgValidSlots = 0;}
    virtual void resized(const boo::SWindowRect &root, const boo::SWindowRect& sub);
    virtual void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}

#endif // SPECTER_VIEW_HPP
