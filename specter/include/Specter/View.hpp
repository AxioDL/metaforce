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

class View
{
    boo::IGraphicsBufferD* m_bgVertBuf;
    boo::IGraphicsBufferD* m_bgInstBuf;
    boo::IVertexFormat* m_bgVtxFmt = nullptr; /* OpenGL only */
    boo::IShaderDataBinding* m_bgShaderBinding;
    Zeus::CVector3f m_bgRect[4];
    Zeus::CColor m_bgColor;
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

    struct VertexBlock
    {
        Zeus::CMatrix4f m_mv;
    };
#define SPECTER_VIEW_VERT_BLOCK_GLSL\
    "uniform SpecterViewBlock\n"\
    "{\n"\
    "    mat4 mv;\n"\
    "};\n"

protected:
    View(ViewSystem& system);
    int m_validDynamicSlots = 0;
    boo::IGraphicsBufferD* m_specterVertBlock;

    boo::SWindowRect m_absWindowRect;
    void bindScissor(boo::IGraphicsCommandQueue* gfxQ) {gfxQ->setScissor(m_absWindowRect);}

public:
    void setBackground(Zeus::CColor color) {m_bgColor = color; m_validDynamicSlots = 0;}
    virtual void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}

#endif // SPECTER_VIEW_HPP
