#include "CParticleSwooshShaders.hpp"
#include "Particle/CParticleSwoosh.hpp"
#include "Particle/CSwooshDescription.hpp"
#include "hecl/Pipeline.hpp"

namespace urde {

boo::ObjToken<boo::IShaderPipeline> CParticleSwooshShaders::m_texZWrite;
boo::ObjToken<boo::IShaderPipeline> CParticleSwooshShaders::m_texNoZWrite;
boo::ObjToken<boo::IShaderPipeline> CParticleSwooshShaders::m_texAdditiveZWrite;
boo::ObjToken<boo::IShaderPipeline> CParticleSwooshShaders::m_texAdditiveNoZWrite;

boo::ObjToken<boo::IShaderPipeline> CParticleSwooshShaders::m_noTexZWrite;
boo::ObjToken<boo::IShaderPipeline> CParticleSwooshShaders::m_noTexNoZWrite;
boo::ObjToken<boo::IShaderPipeline> CParticleSwooshShaders::m_noTexAdditiveZWrite;
boo::ObjToken<boo::IShaderPipeline> CParticleSwooshShaders::m_noTexAdditiveNoZWrite;

static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

void CParticleSwooshShaders::Initialize() {
  m_texZWrite = hecl::conv->convert(Shader_CParticleSwooshShaderTexZWrite{});
  m_texNoZWrite = hecl::conv->convert(Shader_CParticleSwooshShaderTexNoZWrite{});
  m_texAdditiveZWrite = hecl::conv->convert(Shader_CParticleSwooshShaderTexAdditiveZWrite{});
  m_texAdditiveNoZWrite = hecl::conv->convert(Shader_CParticleSwooshShaderTexAdditiveNoZWrite{});
  m_noTexZWrite = hecl::conv->convert(Shader_CParticleSwooshShaderNoTexZWrite{});
  m_noTexNoZWrite = hecl::conv->convert(Shader_CParticleSwooshShaderNoTexNoZWrite{});
  m_noTexAdditiveZWrite = hecl::conv->convert(Shader_CParticleSwooshShaderNoTexAdditiveZWrite{});
  m_noTexAdditiveNoZWrite = hecl::conv->convert(Shader_CParticleSwooshShaderNoTexAdditiveNoZWrite{});
}

void CParticleSwooshShaders::Shutdown() {
  m_texZWrite.reset();
  m_texNoZWrite.reset();
  m_texAdditiveZWrite.reset();
  m_texAdditiveNoZWrite.reset();
  m_noTexZWrite.reset();
  m_noTexNoZWrite.reset();
  m_noTexAdditiveZWrite.reset();
  m_noTexAdditiveNoZWrite.reset();
}

CParticleSwooshShaders::EShaderClass CParticleSwooshShaders::GetShaderClass(CParticleSwoosh& gen) {
  CSwooshDescription* desc = gen.GetDesc();

  if (desc->x3c_TEXR)
    return EShaderClass::Tex;
  else
    return EShaderClass::NoTex;
}

void CParticleSwooshShaders::BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CParticleSwoosh& gen) {
  CSwooshDescription* desc = gen.GetDesc();
  boo::ObjToken<boo::IShaderPipeline> pipeline;

  if (desc->x3c_TEXR) {
    if (desc->x44_31_AALP) {
      if (desc->x45_24_ZBUF)
        pipeline = m_texAdditiveZWrite;
      else
        pipeline = m_texAdditiveNoZWrite;
    } else {
      if (desc->x45_24_ZBUF)
        pipeline = m_texZWrite;
      else
        pipeline = m_texNoZWrite;
    }
  } else {
    if (desc->x44_31_AALP) {
      if (desc->x45_24_ZBUF)
        pipeline = m_noTexAdditiveZWrite;
      else
        pipeline = m_noTexAdditiveNoZWrite;
    } else {
      if (desc->x45_24_ZBUF)
        pipeline = m_noTexZWrite;
      else
        pipeline = m_noTexNoZWrite;
    }
  }

  CUVElement* texr = desc->x3c_TEXR.get();
  boo::ObjToken<boo::ITexture> textures[] = {texr ? texr->GetValueTexture(0).GetObj()->GetBooTexture() : nullptr};

  boo::ObjToken<boo::IGraphicsBuffer> uniforms[] = {gen.m_uniformBuf.get()};
  gen.m_dataBind = ctx.newShaderDataBinding(pipeline, gen.m_vertBuf.get(), nullptr, nullptr, 1, uniforms, nullptr,
                                            texr ? 1 : 0, textures, nullptr, nullptr);
}

} // namespace urde
