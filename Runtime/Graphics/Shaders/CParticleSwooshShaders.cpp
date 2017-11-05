#include "CParticleSwooshShaders.hpp"
#include "Particle/CParticleSwoosh.hpp"
#include "Particle/CSwooshDescription.hpp"

namespace urde
{

boo::ObjToken<boo::IShaderPipeline> CParticleSwooshShaders::m_texZWrite;
boo::ObjToken<boo::IShaderPipeline> CParticleSwooshShaders::m_texNoZWrite;
boo::ObjToken<boo::IShaderPipeline> CParticleSwooshShaders::m_texAdditiveZWrite;
boo::ObjToken<boo::IShaderPipeline> CParticleSwooshShaders::m_texAdditiveNoZWrite;

boo::ObjToken<boo::IShaderPipeline> CParticleSwooshShaders::m_noTexZWrite;
boo::ObjToken<boo::IShaderPipeline> CParticleSwooshShaders::m_noTexNoZWrite;
boo::ObjToken<boo::IShaderPipeline> CParticleSwooshShaders::m_noTexAdditiveZWrite;
boo::ObjToken<boo::IShaderPipeline> CParticleSwooshShaders::m_noTexAdditiveNoZWrite;

boo::ObjToken<boo::IVertexFormat> CParticleSwooshShaders::m_vtxFormat;

CParticleSwooshShaders::EShaderClass CParticleSwooshShaders::GetShaderClass(CParticleSwoosh& gen)
{
    CSwooshDescription* desc = gen.GetDesc();

    if (desc->x3c_TEXR)
        return EShaderClass::Tex;
    else
        return EShaderClass::NoTex;
}

void CParticleSwooshShaders::BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CParticleSwoosh& gen)
{
    CSwooshDescription* desc = gen.GetDesc();
    boo::ObjToken<boo::IShaderPipeline> pipeline;

    if (desc->x3c_TEXR)
    {
        if (desc->x44_31_AALP)
        {
            if (desc->x45_24_ZBUF)
                pipeline = m_texAdditiveZWrite;
            else
                pipeline = m_texAdditiveNoZWrite;
        }
        else
        {
            if (desc->x45_24_ZBUF)
                pipeline = m_texZWrite;
            else
                pipeline = m_texNoZWrite;
        }
    }
    else
    {
        if (desc->x44_31_AALP)
        {
            if (desc->x45_24_ZBUF)
                pipeline = m_noTexAdditiveZWrite;
            else
                pipeline = m_noTexAdditiveNoZWrite;
        }
        else
        {
            if (desc->x45_24_ZBUF)
                pipeline = m_noTexZWrite;
            else
                pipeline = m_noTexNoZWrite;
        }
    }

    CParticleSwooshShaders shad(gen, pipeline);
    TShader<CParticleSwooshShaders>::BuildShaderDataBinding(ctx, shad);
}

URDE_SPECIALIZE_SHADER(CParticleSwooshShaders)

}
