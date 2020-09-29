#include "Runtime/Graphics/Shaders/CParticleSwooshShaders.hpp"

#include <iterator>

#include "Runtime/Particle/CParticleSwoosh.hpp"
#include "Runtime/Particle/CSwooshDescription.hpp"

namespace urde {

CParticleSwooshShaders::EShaderClass CParticleSwooshShaders::GetShaderClass(CParticleSwoosh& gen) {
  CSwooshDescription* desc = gen.GetDesc();

  if (desc->x3c_TEXR)
    return EShaderClass::Tex;
  else
    return EShaderClass::NoTex;
}

void CParticleSwooshShaders::BuildShaderDataBinding(CParticleSwoosh& gen) {
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
  for (size_t i = 0; i < gen.m_dataBind.size(); ++i) {
    gen.m_dataBind[i] =
        ctx.newShaderDataBinding((*pipeline)[i], gen.m_vertBuf.get(), nullptr, nullptr, uniforms.size(),
                                 uniforms.data(), nullptr, texr ? 1 : 0, textures.data(), nullptr, nullptr);
  }
}

} // namespace urde
