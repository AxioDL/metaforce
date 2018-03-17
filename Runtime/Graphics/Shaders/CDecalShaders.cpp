#include "CDecalShaders.hpp"
#include "Particle/CDecal.hpp"

namespace urde
{

boo::ObjToken<boo::IShaderPipeline> CDecalShaders::m_texZTestNoZWrite;
boo::ObjToken<boo::IShaderPipeline> CDecalShaders::m_texAdditiveZTest;
boo::ObjToken<boo::IShaderPipeline> CDecalShaders::m_texRedToAlphaZTest;

boo::ObjToken<boo::IShaderPipeline> CDecalShaders::m_noTexZTestNoZWrite;
boo::ObjToken<boo::IShaderPipeline> CDecalShaders::m_noTexAdditiveZTest;

boo::ObjToken<boo::IVertexFormat> CDecalShaders::m_vtxFormatTex;
boo::ObjToken<boo::IVertexFormat> CDecalShaders::m_vtxFormatNoTex;

void CDecalShaders::BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CQuadDecal& decal)
{
    boo::ObjToken<boo::IShaderPipeline> regPipeline;
    boo::ObjToken<boo::IShaderPipeline> redToAlphaPipeline;

    if (decal.m_desc->x14_TEX)
    {
        if (decal.m_desc->x18_ADD)
            regPipeline = m_texAdditiveZTest;
        else
            regPipeline = m_texZTestNoZWrite;
        redToAlphaPipeline = m_texRedToAlphaZTest;
    }
    else
    {
        if (decal.m_desc->x18_ADD)
            regPipeline = m_noTexAdditiveZTest;
        else
            regPipeline = m_noTexZTestNoZWrite;
    }

    CDecalShaders shad(decal, regPipeline, redToAlphaPipeline);
    TShader<CDecalShaders>::BuildShaderDataBinding(ctx, shad);
}

URDE_SPECIALIZE_SHADER(CDecalShaders)

}
