#include "CElementGenShaders.hpp"
#include "Particle/CElementGen.hpp"

namespace urde
{

boo::IShaderPipeline* CElementGenShaders::m_texZTestZWrite = nullptr;
boo::IShaderPipeline* CElementGenShaders::m_texNoZTestZWrite = nullptr;
boo::IShaderPipeline* CElementGenShaders::m_texZTestNoZWrite = nullptr;
boo::IShaderPipeline* CElementGenShaders::m_texNoZTestNoZWrite = nullptr;
boo::IShaderPipeline* CElementGenShaders::m_texAdditiveZTest = nullptr;
boo::IShaderPipeline* CElementGenShaders::m_texAdditiveNoZTest = nullptr;
boo::IShaderPipeline* CElementGenShaders::m_texRedToAlphaZTest = nullptr;
boo::IShaderPipeline* CElementGenShaders::m_texRedToAlphaNoZTest = nullptr;

boo::IShaderPipeline* CElementGenShaders::m_indTexZWrite = nullptr;
boo::IShaderPipeline* CElementGenShaders::m_indTexNoZWrite = nullptr;
boo::IShaderPipeline* CElementGenShaders::m_indTexAdditive = nullptr;

boo::IShaderPipeline* CElementGenShaders::m_cindTexZWrite = nullptr;
boo::IShaderPipeline* CElementGenShaders::m_cindTexNoZWrite = nullptr;
boo::IShaderPipeline* CElementGenShaders::m_cindTexAdditive = nullptr;

boo::IShaderPipeline* CElementGenShaders::m_noTexZTestZWrite = nullptr;
boo::IShaderPipeline* CElementGenShaders::m_noTexNoZTestZWrite = nullptr;
boo::IShaderPipeline* CElementGenShaders::m_noTexZTestNoZWrite = nullptr;
boo::IShaderPipeline* CElementGenShaders::m_noTexNoZTestNoZWrite = nullptr;
boo::IShaderPipeline* CElementGenShaders::m_noTexAdditiveZTest = nullptr;
boo::IShaderPipeline* CElementGenShaders::m_noTexAdditiveNoZTest = nullptr;

boo::IVertexFormat* CElementGenShaders::m_vtxFormatTex = nullptr;
boo::IVertexFormat* CElementGenShaders::m_vtxFormatIndTex = nullptr;
boo::IVertexFormat* CElementGenShaders::m_vtxFormatNoTex = nullptr;

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
    boo::IShaderPipeline* regPipeline = nullptr;
    boo::IShaderPipeline* redToAlphaPipeline = nullptr;

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
                redToAlphaPipeline = m_texRedToAlphaZTest;
            else
                redToAlphaPipeline = m_texRedToAlphaNoZTest;

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

    CElementGenShaders shad(gen, regPipeline, redToAlphaPipeline);
    TShader<CElementGenShaders>::BuildShaderDataBinding(ctx, shad);
}

void CElementGenShaders::Shutdown() {}

URDE_SPECIALIZE_SHADER(CElementGenShaders)

}
