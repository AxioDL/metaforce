#ifndef __URDE_CDECALSHADERS_HPP__
#define __URDE_CDECALSHADERS_HPP__

#include "TShader.hpp"
#include "Graphics/CGraphics.hpp"
#include "boo/graphicsdev/GL.hpp"
#include "boo/graphicsdev/D3D.hpp"
#include "boo/graphicsdev/Metal.hpp"
#include "boo/graphicsdev/Vulkan.hpp"

namespace urde
{
class CQuadDecal;

class CDecalShaders
{
    friend struct OGLDecalDataBindingFactory;
    friend struct VulkanDecalDataBindingFactory;
    friend struct D3DDecalDataBindingFactory;
    friend struct MetalDecalDataBindingFactory;

private:
    static boo::ObjToken<boo::IShaderPipeline> m_texZTestNoZWrite;
    static boo::ObjToken<boo::IShaderPipeline> m_texAdditiveZTest;
    static boo::ObjToken<boo::IShaderPipeline> m_texRedToAlphaZTest;

    static boo::ObjToken<boo::IShaderPipeline> m_noTexZTestNoZWrite;
    static boo::ObjToken<boo::IShaderPipeline> m_noTexAdditiveZTest;

    static boo::ObjToken<boo::IVertexFormat> m_vtxFormatTex; /* No OpenGL */
    static boo::ObjToken<boo::IVertexFormat> m_vtxFormatNoTex; /* No OpenGL */

    CQuadDecal& m_decal;
    boo::ObjToken<boo::IShaderPipeline> m_regPipeline;
    boo::ObjToken<boo::IShaderPipeline> m_redToAlphaPipeline;
    CDecalShaders(CQuadDecal& decal,
                  const boo::ObjToken<boo::IShaderPipeline>& regPipeline,
                  const boo::ObjToken<boo::IShaderPipeline>& redToAlphaPipeline)
    : m_decal(decal), m_regPipeline(regPipeline), m_redToAlphaPipeline(redToAlphaPipeline) {}

public:
    static void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CQuadDecal& decal);

    using _CLS = CDecalShaders;
#include "TShaderDecl.hpp"
};

}

#endif // __URDE_CDECALSHADERS_HPP__
