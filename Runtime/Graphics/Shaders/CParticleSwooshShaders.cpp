#include "Runtime/Graphics/Shaders/CParticleSwooshShaders.hpp"

#include <iterator>

#include "Runtime/Particle/CParticleSwoosh.hpp"
#include "Runtime/Particle/CSwooshDescription.hpp"

#include <hecl/Pipeline.hpp>

namespace urde {

std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CParticleSwooshShaders::m_texZWrite;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CParticleSwooshShaders::m_texNoZWrite;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CParticleSwooshShaders::m_texAdditiveZWrite;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CParticleSwooshShaders::m_texAdditiveNoZWrite;

std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CParticleSwooshShaders::m_noTexZWrite;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CParticleSwooshShaders::m_noTexNoZWrite;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CParticleSwooshShaders::m_noTexAdditiveZWrite;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CParticleSwooshShaders::m_noTexAdditiveNoZWrite;

static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

void CParticleSwooshShaders::Initialize() {
  m_texZWrite = {hecl::conv->convert(Shader_CParticleSwooshShaderTexZWrite{}),
                 hecl::conv->convert(Shader_CParticleSwooshShaderTexZWriteAWrite{})};
  m_texNoZWrite = {hecl::conv->convert(Shader_CParticleSwooshShaderTexNoZWrite{}),
                   hecl::conv->convert(Shader_CParticleSwooshShaderTexNoZWriteAWrite{})};
  m_texAdditiveZWrite = {hecl::conv->convert(Shader_CParticleSwooshShaderTexAdditiveZWrite{}),
                         hecl::conv->convert(Shader_CParticleSwooshShaderTexAdditiveZWriteAWrite{})};
  m_texAdditiveNoZWrite = {hecl::conv->convert(Shader_CParticleSwooshShaderTexAdditiveNoZWrite{}),
                           hecl::conv->convert(Shader_CParticleSwooshShaderTexAdditiveNoZWriteAWrite{})};
  m_noTexZWrite = {hecl::conv->convert(Shader_CParticleSwooshShaderNoTexZWrite{}),
                   hecl::conv->convert(Shader_CParticleSwooshShaderNoTexZWriteAWrite{})};
  m_noTexNoZWrite = {hecl::conv->convert(Shader_CParticleSwooshShaderNoTexNoZWrite{}),
                     hecl::conv->convert(Shader_CParticleSwooshShaderNoTexNoZWriteAWrite{})};
  m_noTexAdditiveZWrite = {hecl::conv->convert(Shader_CParticleSwooshShaderNoTexAdditiveZWrite{}),
                           hecl::conv->convert(Shader_CParticleSwooshShaderNoTexAdditiveZWriteAWrite{})};
  m_noTexAdditiveNoZWrite = {hecl::conv->convert(Shader_CParticleSwooshShaderNoTexAdditiveNoZWrite{}),
                             hecl::conv->convert(Shader_CParticleSwooshShaderNoTexAdditiveNoZWriteAWrite{})};
}

void CParticleSwooshShaders::Shutdown() {
  for (auto& s : m_texZWrite) s.reset();
  for (auto& s : m_texNoZWrite) s.reset();
  for (auto& s : m_texAdditiveZWrite) s.reset();
  for (auto& s : m_texAdditiveNoZWrite) s.reset();
  for (auto& s : m_noTexZWrite) s.reset();
  for (auto& s : m_noTexNoZWrite) s.reset();
  for (auto& s : m_noTexAdditiveZWrite) s.reset();
  for (auto& s : m_noTexAdditiveNoZWrite) s.reset();
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
  std::array<boo::ObjToken<boo::IShaderPipeline>, 2>* pipeline = nullptr;

  if (desc->x3c_TEXR) {
    if (desc->x44_31_AALP) {
      if (desc->x45_24_ZBUF)
        pipeline = &m_texAdditiveZWrite;
      else
        pipeline = &m_texAdditiveNoZWrite;
    } else {
      if (desc->x45_24_ZBUF)
        pipeline = &m_texZWrite;
      else
        pipeline = &m_texNoZWrite;
    }
  } else {
    if (desc->x44_31_AALP) {
      if (desc->x45_24_ZBUF)
        pipeline = &m_noTexAdditiveZWrite;
      else
        pipeline = &m_noTexAdditiveNoZWrite;
    } else {
      if (desc->x45_24_ZBUF)
        pipeline = &m_noTexZWrite;
      else
        pipeline = &m_noTexNoZWrite;
    }
  }

  const CUVElement* const texr = desc->x3c_TEXR.get();
  const std::array<boo::ObjToken<boo::ITexture>, 1> textures{
      texr ? texr->GetValueTexture(0).GetObj()->GetBooTexture() : nullptr,
  };

  const std::array<boo::ObjToken<boo::IGraphicsBuffer>, 1> uniforms{gen.m_uniformBuf.get()};
  for (size_t i = 0; i < std::size(gen.m_dataBind); ++i) {
    gen.m_dataBind[i] =
        ctx.newShaderDataBinding((*pipeline)[i], gen.m_vertBuf.get(), nullptr, nullptr, uniforms.size(),
                                 uniforms.data(), nullptr, texr ? 1 : 0, textures.data(), nullptr, nullptr);
  }
}

} // namespace urde
