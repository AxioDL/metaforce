#ifndef __URDE_CPARTICLESWOOSHSHADERS_HPP__
#define __URDE_CPARTICLESWOOSHSHADERS_HPP__

#include "TShader.hpp"
#include "Graphics/CGraphics.hpp"
#include "boo/graphicsdev/GL.hpp"
#include "boo/graphicsdev/D3D.hpp"
#include "boo/graphicsdev/Metal.hpp"
#include "boo/graphicsdev/Vulkan.hpp"

namespace urde
{
class CParticleSwoosh;

class CParticleSwooshShaders
{
    friend struct OGLParticleSwooshDataBindingFactory;
    friend struct VulkanParticleSwooshDataBindingFactory;
    friend struct D3DParticleSwooshDataBindingFactory;
    friend struct MetalParticleSwooshDataBindingFactory;
public:
    enum class EShaderClass
    {
        Tex,
        NoTex
    };

    struct Vert
    {
        zeus::CVector3f m_pos;
        zeus::CVector2f m_uv;
        zeus::CColor m_color;
    };

private:
    static boo::IShaderPipeline* m_texZWrite;
    static boo::IShaderPipeline* m_texNoZWrite;
    static boo::IShaderPipeline* m_texAdditiveZWrite;
    static boo::IShaderPipeline* m_texAdditiveNoZWrite;

    static boo::IShaderPipeline* m_noTexZWrite;
    static boo::IShaderPipeline* m_noTexNoZWrite;
    static boo::IShaderPipeline* m_noTexAdditiveZWrite;
    static boo::IShaderPipeline* m_noTexAdditiveNoZWrite;

    static boo::IVertexFormat* m_vtxFormat; /* No OpenGL */

    CParticleSwoosh& m_gen;
    boo::IShaderPipeline* m_pipeline;
    CParticleSwooshShaders(CParticleSwoosh& gen, boo::IShaderPipeline* pipeline)
    : m_gen(gen), m_pipeline(pipeline) {}

public:
    static EShaderClass GetShaderClass(CParticleSwoosh& gen);
    static void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CParticleSwoosh& gen);

    using _CLS = CParticleSwooshShaders;
#include "TShaderDecl.hpp"
};

}

#endif // __URDE_CPARTICLESWOOSHSHADERS_HPP__
