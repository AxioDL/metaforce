#include "CElementGenShaders.hpp"
#include "Particle/CElementGen.hpp"

namespace urde
{

boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_texZTestZWrite;
boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_texNoZTestZWrite;
boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_texZTestNoZWrite;
boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_texNoZTestNoZWrite;
boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_texAdditiveZTest;
boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_texAdditiveNoZTest;
boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_texRedToAlphaZTest;
boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_texRedToAlphaNoZTest;
boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_texZTestNoZWriteSub;
boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_texNoZTestNoZWriteSub;
boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_texRedToAlphaZTestSub;
boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_texRedToAlphaNoZTestSub;

boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_indTexZWrite;
boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_indTexNoZWrite;
boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_indTexAdditive;

boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_cindTexZWrite;
boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_cindTexNoZWrite;
boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_cindTexAdditive;

boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_noTexZTestZWrite;
boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_noTexNoZTestZWrite;
boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_noTexZTestNoZWrite;
boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_noTexNoZTestNoZWrite;
boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_noTexAdditiveZTest;
boo::ObjToken<boo::IShaderPipeline> CElementGenShaders::m_noTexAdditiveNoZTest;

boo::ObjToken<boo::IVertexFormat> CElementGenShaders::m_vtxFormatTex;
boo::ObjToken<boo::IVertexFormat> CElementGenShaders::m_vtxFormatIndTex;
boo::ObjToken<boo::IVertexFormat> CElementGenShaders::m_vtxFormatNoTex;

CElementGenShaders::EShaderClass CElementGenShaders::GetShaderClass(CElementGen& gen)
{
    CGenDescription* desc = gen.x1c_genDesc.GetObj();

    if (desc->x54_x40_TEXR)
    {
        if (desc->x58_x44_TIND)
            return EShaderClass::IndTex;
        else
            return EShaderClass::Tex;
    }
    else
        return EShaderClass::NoTex;
}

void CElementGenShaders::BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CElementGen& gen)
{
    CGenDescription* desc = gen.x1c_genDesc.GetObj();
    boo::ObjToken<boo::IShaderPipeline> regPipeline;
    boo::ObjToken<boo::IShaderPipeline> regPipelineSub;
    boo::ObjToken<boo::IShaderPipeline> redToAlphaPipeline;
    boo::ObjToken<boo::IShaderPipeline> redToAlphaPipelineSub;
    boo::ObjToken<boo::IShaderPipeline> regPipelinePmus;
    boo::ObjToken<boo::IShaderPipeline> redToAlphaPipelinePmus;

    if (desc->x54_x40_TEXR)
    {
        if (desc->x58_x44_TIND)
        {
            if (desc->x45_30_x32_24_CIND)
            {
                if (gen.x26c_26_AAPH)
                    regPipeline = m_cindTexAdditive;
                else
                {
                    if (gen.x26c_27_ZBUF)
                        regPipeline = m_cindTexZWrite;
                    else
                        regPipeline = m_cindTexNoZWrite;
                }
            }
            else
            {
                if (gen.x26c_26_AAPH)
                    regPipeline = m_indTexAdditive;
                else
                {
                    if (gen.x26c_27_ZBUF)
                        regPipeline = m_indTexZWrite;
                    else
                        regPipeline = m_indTexNoZWrite;
                }
            }
        }
        else
        {
            if (gen.x26c_28_zTest)
            {
                redToAlphaPipeline = m_texRedToAlphaZTest;
                regPipelineSub = m_texZTestNoZWriteSub;
                redToAlphaPipelineSub = m_texRedToAlphaZTestSub;
            }
            else
            {
                redToAlphaPipeline = m_texRedToAlphaNoZTest;
                regPipelineSub = m_texNoZTestNoZWriteSub;
                redToAlphaPipelineSub = m_texRedToAlphaNoZTestSub;
            }

            if (gen.x26c_26_AAPH)
            {
                if (gen.x26c_28_zTest)
                    regPipeline = m_texAdditiveZTest;
                else
                    regPipeline = m_texAdditiveNoZTest;
            }
            else
            {
                if (gen.x26c_28_zTest)
                {
                    if (gen.x26c_27_ZBUF)
                        regPipeline = m_texZTestZWrite;
                    else
                        regPipeline = m_texZTestNoZWrite;
                }
                else
                {
                    if (gen.x26c_27_ZBUF)
                        regPipeline = m_texNoZTestZWrite;
                    else
                        regPipeline = m_texNoZTestNoZWrite;
                }
            }
        }
    }
    else
    {
        if (gen.x26c_26_AAPH)
        {
            if (gen.x26c_28_zTest)
                regPipeline = m_noTexAdditiveZTest;
            else
                regPipeline = m_noTexAdditiveNoZTest;
        }
        else
        {
            if (gen.x26c_28_zTest)
            {
                if (gen.x26c_27_ZBUF)
                    regPipeline = m_noTexZTestZWrite;
                else
                    regPipeline = m_noTexZTestNoZWrite;
            }
            else
            {
                if (gen.x26c_27_ZBUF)
                    regPipeline = m_noTexNoZTestZWrite;
                else
                    regPipeline = m_noTexNoZTestNoZWrite;
            }
        }
    }

    if (desc->x45_24_x31_26_PMUS)
    {
        if (desc->x54_x40_TEXR)
        {
            redToAlphaPipelinePmus = m_texRedToAlphaZTest;
            if (desc->x44_31_x31_25_PMAB)
                regPipelinePmus = m_texAdditiveZTest;
            else
                regPipelinePmus = m_texZTestZWrite;
        }
        else
        {
            if (desc->x44_31_x31_25_PMAB)
                regPipelinePmus = m_noTexAdditiveZTest;
            else
                regPipelinePmus = m_noTexZTestZWrite;
        }
    }

    CElementGenShaders shad(gen, regPipeline, regPipelineSub, redToAlphaPipeline, redToAlphaPipelineSub,
                            regPipelinePmus, redToAlphaPipelinePmus);
    TShader<CElementGenShaders>::BuildShaderDataBinding(ctx, shad);
}

URDE_SPECIALIZE_SHADER(CElementGenShaders)

}
