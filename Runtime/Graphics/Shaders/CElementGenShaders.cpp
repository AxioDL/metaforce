#include "CElementGenShaders.hpp"
#include "Particle/CElementGen.hpp"
#include "hecl/Pipeline.hpp"

namespace urde {

std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_texZTestZWrite;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_texNoZTestZWrite;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_texZTestNoZWrite;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_texNoZTestNoZWrite;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_texAdditiveZTest;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_texAdditiveNoZTest;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_texRedToAlphaZTest;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_texRedToAlphaNoZTest;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_texZTestNoZWriteSub;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_texNoZTestNoZWriteSub;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_texRedToAlphaZTestSub;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_texRedToAlphaNoZTestSub;

std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_indTexZWrite;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_indTexNoZWrite;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_indTexAdditive;

std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_cindTexZWrite;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_cindTexNoZWrite;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_cindTexAdditive;

std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_noTexZTestZWrite;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_noTexNoZTestZWrite;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_noTexZTestNoZWrite;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_noTexNoZTestNoZWrite;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_noTexAdditiveZTest;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CElementGenShaders::m_noTexAdditiveNoZTest;

void CElementGenShaders::Initialize() {
  m_texZTestZWrite = {hecl::conv->convert(Shader_CElementGenShaderTexZTestZWrite{}),
                      hecl::conv->convert(Shader_CElementGenShaderTexZTestZWrite{})};
  m_texNoZTestZWrite = {hecl::conv->convert(Shader_CElementGenShaderTexNoZTestZWrite{}),
                        hecl::conv->convert(Shader_CElementGenShaderTexNoZTestZWriteAWrite{})};
  m_texZTestNoZWrite = {hecl::conv->convert(Shader_CElementGenShaderTexZTestNoZWrite{}),
                        hecl::conv->convert(Shader_CElementGenShaderTexZTestNoZWriteAWrite{})};
  m_texNoZTestNoZWrite = {hecl::conv->convert(Shader_CElementGenShaderTexNoZTestNoZWrite{}),
                          hecl::conv->convert(Shader_CElementGenShaderTexNoZTestNoZWriteAWrite{})};
  m_texAdditiveZTest = {hecl::conv->convert(Shader_CElementGenShaderTexAdditiveZTest{}),
                        hecl::conv->convert(Shader_CElementGenShaderTexAdditiveZTestAWrite{})};
  m_texAdditiveNoZTest = {hecl::conv->convert(Shader_CElementGenShaderTexAdditiveNoZTest{}),
                          hecl::conv->convert(Shader_CElementGenShaderTexAdditiveNoZTestAWrite{})};
  m_texRedToAlphaZTest = {hecl::conv->convert(Shader_CElementGenShaderTexRedToAlphaZTest{}),
                          hecl::conv->convert(Shader_CElementGenShaderTexRedToAlphaZTestAWrite{})};
  m_texRedToAlphaNoZTest = {hecl::conv->convert(Shader_CElementGenShaderTexRedToAlphaNoZTest{}),
                            hecl::conv->convert(Shader_CElementGenShaderTexRedToAlphaNoZTestAWrite{})};
  m_texZTestNoZWriteSub = {hecl::conv->convert(Shader_CElementGenShaderTexZTestNoZWriteSub{}),
                           hecl::conv->convert(Shader_CElementGenShaderTexZTestNoZWriteSubAWrite{})};
  m_texNoZTestNoZWriteSub = {hecl::conv->convert(Shader_CElementGenShaderTexNoZTestNoZWriteSub{}),
                             hecl::conv->convert(Shader_CElementGenShaderTexNoZTestNoZWriteSubAWrite{})};
  m_texRedToAlphaZTestSub = {hecl::conv->convert(Shader_CElementGenShaderTexRedToAlphaZTestSub{}),
                             hecl::conv->convert(Shader_CElementGenShaderTexRedToAlphaZTestSubAWrite{})};
  m_texRedToAlphaNoZTestSub = {hecl::conv->convert(Shader_CElementGenShaderTexRedToAlphaNoZTestSub{}),
                               hecl::conv->convert(Shader_CElementGenShaderTexRedToAlphaNoZTestSubAWrite{})};

  m_indTexZWrite = {hecl::conv->convert(Shader_CElementGenShaderIndTexZWrite{}),
                    hecl::conv->convert(Shader_CElementGenShaderIndTexZWriteAWrite{})};
  m_indTexNoZWrite = {hecl::conv->convert(Shader_CElementGenShaderIndTexNoZWrite{}),
                      hecl::conv->convert(Shader_CElementGenShaderIndTexNoZWriteAWrite{})};
  m_indTexAdditive = {hecl::conv->convert(Shader_CElementGenShaderIndTexAdditive{}),
                      hecl::conv->convert(Shader_CElementGenShaderIndTexAdditiveAWrite{})};

  m_cindTexZWrite = {hecl::conv->convert(Shader_CElementGenShaderCindTexZWrite{}),
                     hecl::conv->convert(Shader_CElementGenShaderCindTexZWriteAWrite{})};
  m_cindTexNoZWrite = {hecl::conv->convert(Shader_CElementGenShaderCindTexNoZWrite{}),
                       hecl::conv->convert(Shader_CElementGenShaderCindTexNoZWriteAWrite{})};
  m_cindTexAdditive = {hecl::conv->convert(Shader_CElementGenShaderCindTexAdditive{}),
                       hecl::conv->convert(Shader_CElementGenShaderCindTexAdditiveAWrite{})};

  m_noTexZTestZWrite = {hecl::conv->convert(Shader_CElementGenShaderNoTexZTestZWrite{}),
                        hecl::conv->convert(Shader_CElementGenShaderNoTexZTestZWriteAWrite{})};
  m_noTexNoZTestZWrite = {hecl::conv->convert(Shader_CElementGenShaderNoTexNoZTestZWrite{}),
                          hecl::conv->convert(Shader_CElementGenShaderNoTexNoZTestZWriteAWrite{})};
  m_noTexZTestNoZWrite = {hecl::conv->convert(Shader_CElementGenShaderNoTexZTestNoZWrite{}),
                          hecl::conv->convert(Shader_CElementGenShaderNoTexZTestNoZWriteAWrite{})};
  m_noTexNoZTestNoZWrite = {hecl::conv->convert(Shader_CElementGenShaderNoTexNoZTestNoZWrite{}),
                            hecl::conv->convert(Shader_CElementGenShaderNoTexNoZTestNoZWriteAWrite{})};
  m_noTexAdditiveZTest = {hecl::conv->convert(Shader_CElementGenShaderNoTexAdditiveZTest{}),
                          hecl::conv->convert(Shader_CElementGenShaderNoTexAdditiveZTestAWrite{})};
  m_noTexAdditiveNoZTest = {hecl::conv->convert(Shader_CElementGenShaderNoTexAdditiveNoZTest{}),
                            hecl::conv->convert(Shader_CElementGenShaderNoTexAdditiveNoZTestAWrite{})};
}

void CElementGenShaders::Shutdown() {
  for (auto& s : m_texZTestZWrite) s.reset();
  for (auto& s : m_texNoZTestZWrite) s.reset();
  for (auto& s : m_texZTestNoZWrite) s.reset();
  for (auto& s : m_texNoZTestNoZWrite) s.reset();
  for (auto& s : m_texAdditiveZTest) s.reset();
  for (auto& s : m_texAdditiveNoZTest) s.reset();
  for (auto& s : m_texRedToAlphaZTest) s.reset();
  for (auto& s : m_texRedToAlphaNoZTest) s.reset();
  for (auto& s : m_texZTestNoZWriteSub) s.reset();
  for (auto& s : m_texNoZTestNoZWriteSub) s.reset();
  for (auto& s : m_texRedToAlphaZTestSub) s.reset();
  for (auto& s : m_texRedToAlphaNoZTestSub) s.reset();

  for (auto& s : m_indTexZWrite) s.reset();
  for (auto& s : m_indTexNoZWrite) s.reset();
  for (auto& s : m_indTexAdditive) s.reset();

  for (auto& s : m_cindTexZWrite) s.reset();
  for (auto& s : m_cindTexNoZWrite) s.reset();
  for (auto& s : m_cindTexAdditive) s.reset();

  for (auto& s : m_noTexZTestZWrite) s.reset();
  for (auto& s : m_noTexNoZTestZWrite) s.reset();
  for (auto& s : m_noTexZTestNoZWrite) s.reset();
  for (auto& s : m_noTexNoZTestNoZWrite) s.reset();
  for (auto& s : m_noTexAdditiveZTest) s.reset();
  for (auto& s : m_noTexAdditiveNoZTest) s.reset();
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
  std::array<boo::ObjToken<boo::IShaderPipeline>, 2>* regPipeline = nullptr;
  std::array<boo::ObjToken<boo::IShaderPipeline>, 2>* regPipelineSub = nullptr;
  std::array<boo::ObjToken<boo::IShaderPipeline>, 2>* redToAlphaPipeline = nullptr;
  std::array<boo::ObjToken<boo::IShaderPipeline>, 2>* redToAlphaPipelineSub = nullptr;
  std::array<boo::ObjToken<boo::IShaderPipeline>, 2>* regPipelinePmus = nullptr;
  std::array<boo::ObjToken<boo::IShaderPipeline>, 2>* redToAlphaPipelinePmus = nullptr;

  if (desc->x54_x40_TEXR) {
    if (desc->x58_x44_TIND) {
      if (desc->x45_30_x32_24_CIND) {
        if (gen.x26c_26_AAPH)
          regPipeline = &m_cindTexAdditive;
        else {
          if (gen.x26c_27_ZBUF)
            regPipeline = &m_cindTexZWrite;
          else
            regPipeline = &m_cindTexNoZWrite;
        }
      } else {
        if (gen.x26c_26_AAPH)
          regPipeline = &m_indTexAdditive;
        else {
          if (gen.x26c_27_ZBUF)
            regPipeline = &m_indTexZWrite;
          else
            regPipeline = &m_indTexNoZWrite;
        }
      }
    } else {
      if (gen.x26c_28_zTest) {
        redToAlphaPipeline = &m_texRedToAlphaZTest;
        regPipelineSub = &m_texZTestNoZWriteSub;
        redToAlphaPipelineSub = &m_texRedToAlphaZTestSub;
      } else {
        redToAlphaPipeline = &m_texRedToAlphaNoZTest;
        regPipelineSub = &m_texNoZTestNoZWriteSub;
        redToAlphaPipelineSub = &m_texRedToAlphaNoZTestSub;
      }

      if (gen.x26c_26_AAPH) {
        if (gen.x26c_28_zTest)
          regPipeline = &m_texAdditiveZTest;
        else
          regPipeline = &m_texAdditiveNoZTest;
      } else {
        if (gen.x26c_28_zTest) {
          if (gen.x26c_27_ZBUF)
            regPipeline = &m_texZTestZWrite;
          else
            regPipeline = &m_texZTestNoZWrite;
        } else {
          if (gen.x26c_27_ZBUF)
            regPipeline = &m_texNoZTestZWrite;
          else
            regPipeline = &m_texNoZTestNoZWrite;
        }
      }
    }
  } else {
    if (gen.x26c_26_AAPH) {
      if (gen.x26c_28_zTest)
        regPipeline = &m_noTexAdditiveZTest;
      else
        regPipeline = &m_noTexAdditiveNoZTest;
    } else {
      if (gen.x26c_28_zTest) {
        if (gen.x26c_27_ZBUF)
          regPipeline = &m_noTexZTestZWrite;
        else
          regPipeline = &m_noTexZTestNoZWrite;
      } else {
        if (gen.x26c_27_ZBUF)
          regPipeline = &m_noTexNoZTestZWrite;
        else
          regPipeline = &m_noTexNoZTestNoZWrite;
      }
    }
  }

  if (desc->x45_24_x31_26_PMUS) {
    if (desc->x54_x40_TEXR) {
      redToAlphaPipelinePmus = &m_texRedToAlphaZTest;
      if (desc->x44_31_x31_25_PMAB)
        regPipelinePmus = &m_texAdditiveZTest;
      else
        regPipelinePmus = &m_texZTestZWrite;
    } else {
      if (desc->x44_31_x31_25_PMAB)
        regPipelinePmus = &m_noTexAdditiveZTest;
      else
        regPipelinePmus = &m_noTexZTestZWrite;
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
      for (int i = 0; i < 2; ++i)
        gen.m_normalDataBind[i] = ctx.newShaderDataBinding((*regPipeline)[i], nullptr, gen.m_instBuf.get(), nullptr, 1,
                                                           uniforms, nullptr, texCount, textures, nullptr, nullptr);
    if (regPipelineSub)
      for (int i = 0; i < 2; ++i)
        gen.m_normalSubDataBind[i] = ctx.newShaderDataBinding((*regPipelineSub)[i], nullptr, gen.m_instBuf.get(),
                                                              nullptr, 1, uniforms, nullptr, texCount, textures,
                                                              nullptr, nullptr);
    if (redToAlphaPipeline)
      for (int i = 0; i < 2; ++i)
        gen.m_redToAlphaDataBind[i] = ctx.newShaderDataBinding((*redToAlphaPipeline)[i], nullptr, gen.m_instBuf.get(),
                                                               nullptr, 1, uniforms, nullptr, texCount, textures,
                                                               nullptr, nullptr);
    if (redToAlphaPipelineSub)
      for (int i = 0; i < 2; ++i)
        gen.m_redToAlphaSubDataBind[i] =
          ctx.newShaderDataBinding((*redToAlphaPipelineSub)[i], nullptr, gen.m_instBuf.get(), nullptr, 1, uniforms,
                                   nullptr, texCount, textures, nullptr, nullptr);
  }

  if (gen.m_instBufPmus) {
    boo::ObjToken<boo::IGraphicsBuffer> uniforms[] = {gen.m_uniformBufPmus.get()};
    texCount = std::min(texCount, 1);

    if (regPipelinePmus)
      for (int i = 0; i < 2; ++i)
        gen.m_normalDataBindPmus[i] = ctx.newShaderDataBinding((*regPipelinePmus)[i], nullptr, gen.m_instBufPmus.get(),
                                                               nullptr, 1, uniforms, nullptr, texCount, textures,
                                                               nullptr, nullptr);
    if (redToAlphaPipelinePmus)
      for (int i = 0; i < 2; ++i)
        gen.m_redToAlphaDataBindPmus[i] =
          ctx.newShaderDataBinding((*redToAlphaPipelinePmus)[i], nullptr, gen.m_instBufPmus.get(), nullptr, 1, uniforms,
                                   nullptr, texCount, textures, nullptr, nullptr);
  }
}

} // namespace urde
