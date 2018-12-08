#include "CElementGenShaders.hpp"
#include "Particle/CElementGen.hpp"
#include "hecl/Pipeline.hpp"

namespace urde {

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

void CElementGenShaders::Initialize() {
  m_texZTestZWrite = hecl::conv->convert(Shader_CElementGenShaderTexZTestZWrite{});
  m_texNoZTestZWrite = hecl::conv->convert(Shader_CElementGenShaderTexNoZTestZWrite{});
  m_texZTestNoZWrite = hecl::conv->convert(Shader_CElementGenShaderTexZTestNoZWrite{});
  m_texNoZTestNoZWrite = hecl::conv->convert(Shader_CElementGenShaderTexNoZTestNoZWrite{});
  m_texAdditiveZTest = hecl::conv->convert(Shader_CElementGenShaderTexAdditiveZTest{});
  m_texAdditiveNoZTest = hecl::conv->convert(Shader_CElementGenShaderTexAdditiveNoZTest{});
  m_texRedToAlphaZTest = hecl::conv->convert(Shader_CElementGenShaderTexRedToAlphaZTest{});
  m_texRedToAlphaNoZTest = hecl::conv->convert(Shader_CElementGenShaderTexRedToAlphaNoZTest{});
  m_texZTestNoZWriteSub = hecl::conv->convert(Shader_CElementGenShaderTexZTestNoZWriteSub{});
  m_texNoZTestNoZWriteSub = hecl::conv->convert(Shader_CElementGenShaderTexNoZTestNoZWriteSub{});
  m_texRedToAlphaZTestSub = hecl::conv->convert(Shader_CElementGenShaderTexRedToAlphaZTestSub{});
  m_texRedToAlphaNoZTestSub = hecl::conv->convert(Shader_CElementGenShaderTexRedToAlphaNoZTestSub{});

  m_indTexZWrite = hecl::conv->convert(Shader_CElementGenShaderIndTexZWrite{});
  m_indTexNoZWrite = hecl::conv->convert(Shader_CElementGenShaderIndTexNoZWrite{});
  m_indTexAdditive = hecl::conv->convert(Shader_CElementGenShaderIndTexAdditive{});

  m_cindTexZWrite = hecl::conv->convert(Shader_CElementGenShaderCindTexZWrite{});
  m_cindTexNoZWrite = hecl::conv->convert(Shader_CElementGenShaderCindTexNoZWrite{});
  m_cindTexAdditive = hecl::conv->convert(Shader_CElementGenShaderCindTexAdditive{});

  m_noTexZTestZWrite = hecl::conv->convert(Shader_CElementGenShaderNoTexZTestZWrite{});
  m_noTexNoZTestZWrite = hecl::conv->convert(Shader_CElementGenShaderNoTexNoZTestZWrite{});
  m_noTexZTestNoZWrite = hecl::conv->convert(Shader_CElementGenShaderNoTexZTestNoZWrite{});
  m_noTexNoZTestNoZWrite = hecl::conv->convert(Shader_CElementGenShaderNoTexNoZTestNoZWrite{});
  m_noTexAdditiveZTest = hecl::conv->convert(Shader_CElementGenShaderNoTexAdditiveZTest{});
  m_noTexAdditiveNoZTest = hecl::conv->convert(Shader_CElementGenShaderNoTexAdditiveNoZTest{});
}

void CElementGenShaders::Shutdown() {
  m_texZTestZWrite.reset();
  m_texNoZTestZWrite.reset();
  m_texZTestNoZWrite.reset();
  m_texNoZTestNoZWrite.reset();
  m_texAdditiveZTest.reset();
  m_texAdditiveNoZTest.reset();
  m_texRedToAlphaZTest.reset();
  m_texRedToAlphaNoZTest.reset();
  m_texZTestNoZWriteSub.reset();
  m_texNoZTestNoZWriteSub.reset();
  m_texRedToAlphaZTestSub.reset();
  m_texRedToAlphaNoZTestSub.reset();

  m_indTexZWrite.reset();
  m_indTexNoZWrite.reset();
  m_indTexAdditive.reset();

  m_cindTexZWrite.reset();
  m_cindTexNoZWrite.reset();
  m_cindTexAdditive.reset();

  m_noTexZTestZWrite.reset();
  m_noTexNoZTestZWrite.reset();
  m_noTexZTestNoZWrite.reset();
  m_noTexNoZTestNoZWrite.reset();
  m_noTexAdditiveZTest.reset();
  m_noTexAdditiveNoZTest.reset();
}

CElementGenShaders::EShaderClass CElementGenShaders::GetShaderClass(CElementGen& gen) {
  CGenDescription* desc = gen.x1c_genDesc.GetObj();

  if (desc->x54_x40_TEXR) {
    if (desc->x58_x44_TIND)
      return EShaderClass::IndTex;
    else
      return EShaderClass::Tex;
  } else
    return EShaderClass::NoTex;
}

void CElementGenShaders::BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CElementGen& gen) {
  CGenDescription* desc = gen.x1c_genDesc.GetObj();
  boo::ObjToken<boo::IShaderPipeline> regPipeline;
  boo::ObjToken<boo::IShaderPipeline> regPipelineSub;
  boo::ObjToken<boo::IShaderPipeline> redToAlphaPipeline;
  boo::ObjToken<boo::IShaderPipeline> redToAlphaPipelineSub;
  boo::ObjToken<boo::IShaderPipeline> regPipelinePmus;
  boo::ObjToken<boo::IShaderPipeline> redToAlphaPipelinePmus;

  if (desc->x54_x40_TEXR) {
    if (desc->x58_x44_TIND) {
      if (desc->x45_30_x32_24_CIND) {
        if (gen.x26c_26_AAPH)
          regPipeline = m_cindTexAdditive;
        else {
          if (gen.x26c_27_ZBUF)
            regPipeline = m_cindTexZWrite;
          else
            regPipeline = m_cindTexNoZWrite;
        }
      } else {
        if (gen.x26c_26_AAPH)
          regPipeline = m_indTexAdditive;
        else {
          if (gen.x26c_27_ZBUF)
            regPipeline = m_indTexZWrite;
          else
            regPipeline = m_indTexNoZWrite;
        }
      }
    } else {
      if (gen.x26c_28_zTest) {
        redToAlphaPipeline = m_texRedToAlphaZTest;
        regPipelineSub = m_texZTestNoZWriteSub;
        redToAlphaPipelineSub = m_texRedToAlphaZTestSub;
      } else {
        redToAlphaPipeline = m_texRedToAlphaNoZTest;
        regPipelineSub = m_texNoZTestNoZWriteSub;
        redToAlphaPipelineSub = m_texRedToAlphaNoZTestSub;
      }

      if (gen.x26c_26_AAPH) {
        if (gen.x26c_28_zTest)
          regPipeline = m_texAdditiveZTest;
        else
          regPipeline = m_texAdditiveNoZTest;
      } else {
        if (gen.x26c_28_zTest) {
          if (gen.x26c_27_ZBUF)
            regPipeline = m_texZTestZWrite;
          else
            regPipeline = m_texZTestNoZWrite;
        } else {
          if (gen.x26c_27_ZBUF)
            regPipeline = m_texNoZTestZWrite;
          else
            regPipeline = m_texNoZTestNoZWrite;
        }
      }
    }
  } else {
    if (gen.x26c_26_AAPH) {
      if (gen.x26c_28_zTest)
        regPipeline = m_noTexAdditiveZTest;
      else
        regPipeline = m_noTexAdditiveNoZTest;
    } else {
      if (gen.x26c_28_zTest) {
        if (gen.x26c_27_ZBUF)
          regPipeline = m_noTexZTestZWrite;
        else
          regPipeline = m_noTexZTestNoZWrite;
      } else {
        if (gen.x26c_27_ZBUF)
          regPipeline = m_noTexNoZTestZWrite;
        else
          regPipeline = m_noTexNoZTestNoZWrite;
      }
    }
  }

  if (desc->x45_24_x31_26_PMUS) {
    if (desc->x54_x40_TEXR) {
      redToAlphaPipelinePmus = m_texRedToAlphaZTest;
      if (desc->x44_31_x31_25_PMAB)
        regPipelinePmus = m_texAdditiveZTest;
      else
        regPipelinePmus = m_texZTestZWrite;
    } else {
      if (desc->x44_31_x31_25_PMAB)
        regPipelinePmus = m_noTexAdditiveZTest;
      else
        regPipelinePmus = m_noTexZTestZWrite;
    }
  }

  CUVElement* texr = desc->x54_x40_TEXR.get();
  CUVElement* tind = desc->x58_x44_TIND.get();
  int texCount = 0;
  boo::ObjToken<boo::ITexture> textures[3];

  if (texr) {
    textures[0] = texr->GetValueTexture(0).GetObj()->GetBooTexture();
    texCount = 1;
    if (gen.m_instBuf) {
      if (tind) {
        textures[1] = CGraphics::g_SpareTexture.get();
        textures[2] = tind->GetValueTexture(0).GetObj()->GetBooTexture();
        texCount = 3;
      }
    }
  }

  if (gen.m_instBuf) {
    boo::ObjToken<boo::IGraphicsBuffer> uniforms[] = {gen.m_uniformBuf.get()};

    if (regPipeline)
      gen.m_normalDataBind = ctx.newShaderDataBinding(regPipeline, nullptr, gen.m_instBuf.get(), nullptr, 1, uniforms,
                                                      nullptr, texCount, textures, nullptr, nullptr);
    if (regPipelineSub)
      gen.m_normalSubDataBind = ctx.newShaderDataBinding(regPipelineSub, nullptr, gen.m_instBuf.get(), nullptr, 1,
                                                         uniforms, nullptr, texCount, textures, nullptr, nullptr);
    if (redToAlphaPipeline)
      gen.m_redToAlphaDataBind = ctx.newShaderDataBinding(redToAlphaPipeline, nullptr, gen.m_instBuf.get(), nullptr, 1,
                                                          uniforms, nullptr, texCount, textures, nullptr, nullptr);
    if (redToAlphaPipelineSub)
      gen.m_redToAlphaSubDataBind =
          ctx.newShaderDataBinding(redToAlphaPipelineSub, nullptr, gen.m_instBuf.get(), nullptr, 1, uniforms, nullptr,
                                   texCount, textures, nullptr, nullptr);
  }

  if (gen.m_instBufPmus) {
    boo::ObjToken<boo::IGraphicsBuffer> uniforms[] = {gen.m_uniformBufPmus.get()};
    texCount = std::min(texCount, 1);

    if (regPipelinePmus)
      gen.m_normalDataBindPmus = ctx.newShaderDataBinding(regPipelinePmus, nullptr, gen.m_instBufPmus.get(), nullptr, 1,
                                                          uniforms, nullptr, texCount, textures, nullptr, nullptr);
    if (redToAlphaPipelinePmus)
      gen.m_redToAlphaDataBindPmus =
          ctx.newShaderDataBinding(redToAlphaPipelinePmus, nullptr, gen.m_instBufPmus.get(), nullptr, 1, uniforms,
                                   nullptr, texCount, textures, nullptr, nullptr);
  }
}

} // namespace urde
