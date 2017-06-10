#include "CParticleSwooshShaders.hpp"
#include "Particle/CParticleSwoosh.hpp"
#include "Particle/CSwooshDescription.hpp"

namespace urde
{

boo::IShaderPipeline* CParticleSwooshShaders::m_texZWrite = nullptr;
boo::IShaderPipeline* CParticleSwooshShaders::m_texNoZWrite = nullptr;
boo::IShaderPipeline* CParticleSwooshShaders::m_texAdditiveZWrite = nullptr;
boo::IShaderPipeline* CParticleSwooshShaders::m_texAdditiveNoZWrite = nullptr;

boo::IShaderPipeline* CParticleSwooshShaders::m_noTexZWrite = nullptr;
boo::IShaderPipeline* CParticleSwooshShaders::m_noTexNoZWrite = nullptr;
boo::IShaderPipeline* CParticleSwooshShaders::m_noTexAdditiveZWrite = nullptr;
boo::IShaderPipeline* CParticleSwooshShaders::m_noTexAdditiveNoZWrite = nullptr;

boo::IVertexFormat* CParticleSwooshShaders::m_vtxFormat = nullptr;

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
    boo::IShaderPipeline* pipeline = nullptr;

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

void CParticleSwooshShaders::Shutdown() {}

URDE_SPECIALIZE_SHADER(CParticleSwooshShaders)

}
