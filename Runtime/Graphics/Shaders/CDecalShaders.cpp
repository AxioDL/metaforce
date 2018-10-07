#include "CDecalShaders.hpp"
#include "Particle/CDecal.hpp"
#include "hecl/Pipeline.hpp"

namespace urde
{

boo::ObjToken<boo::IShaderPipeline> CDecalShaders::m_texZTestNoZWrite;
boo::ObjToken<boo::IShaderPipeline> CDecalShaders::m_texAdditiveZTest;
boo::ObjToken<boo::IShaderPipeline> CDecalShaders::m_texRedToAlphaZTest;

boo::ObjToken<boo::IShaderPipeline> CDecalShaders::m_noTexZTestNoZWrite;
boo::ObjToken<boo::IShaderPipeline> CDecalShaders::m_noTexAdditiveZTest;

void CDecalShaders::Initialize()
{
    m_texZTestNoZWrite = hecl::conv->convert(Shader_CDecalShaderTexZTest{});
    m_texAdditiveZTest = hecl::conv->convert(Shader_CDecalShaderTexAdditiveZTest{});
    m_texRedToAlphaZTest = hecl::conv->convert(Shader_CDecalShaderTexRedToAlphaZTest{});
    m_noTexZTestNoZWrite = hecl::conv->convert(Shader_CDecalShaderNoTexZTest{});
    m_noTexAdditiveZTest = hecl::conv->convert(Shader_CDecalShaderNoTexAdditiveZTest{});
}

void CDecalShaders::Shutdown()
{
    m_texZTestNoZWrite.reset();
    m_texAdditiveZTest.reset();
    m_texRedToAlphaZTest.reset();
    m_noTexZTestNoZWrite.reset();
    m_noTexAdditiveZTest.reset();
}

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

    const SQuadDescr* desc = decal.m_desc;

    CUVElement* texr = desc->x14_TEX.get();
    int texCount = 0;
    boo::ObjToken<boo::ITexture> textures[1];

    if (texr)
    {
        textures[0] = texr->GetValueTexture(0).GetObj()->GetBooTexture();
        texCount = 1;
    }

    if (decal.m_instBuf)
    {
        boo::ObjToken<boo::IGraphicsBuffer> uniforms[] = {decal.m_uniformBuf.get()};

        if (regPipeline)
            decal.m_normalDataBind = ctx.newShaderDataBinding(regPipeline, nullptr,
                                                              decal.m_instBuf.get(), nullptr, 1, uniforms,
                                                              nullptr, texCount, textures, nullptr, nullptr);
        if (redToAlphaPipeline)
            decal.m_redToAlphaDataBind = ctx.newShaderDataBinding(redToAlphaPipeline, nullptr,
                                                                  decal.m_instBuf.get(), nullptr, 1, uniforms,
                                                                  nullptr, texCount, textures, nullptr, nullptr);
    }
}

}
