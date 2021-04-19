#include "Runtime/Graphics/Shaders/CParticleSwooshShaders.hpp"

#include <iterator>

#include "Runtime/Particle/CParticleSwoosh.hpp"
#include "Runtime/Particle/CSwooshDescription.hpp"

#include "CParticleSwooshShaders.cpp.hshhead"

namespace metaforce {
using namespace hsh::pipeline;

template <bool Additive, bool AlphaWrite, bool ZWrite>
struct CParticleSwooshShadersTexPipeline
: pipeline<topology<hsh::TriangleStrip>,
           std::conditional_t<Additive, AdditiveAttachment<AlphaWrite>, BlendAttachment<AlphaWrite>>,
           depth_compare<hsh::LEqual>, depth_write<ZWrite>> {
  CParticleSwooshShadersTexPipeline(hsh::vertex_buffer<CParticleSwooshShaders::Vert> vbo,
                                    hsh::uniform_buffer<CParticleSwooshShaders::Uniform> uniBuf, hsh::texture2d tex) {
    this->position = uniBuf->m_xf * hsh::float4(vbo->m_pos, 1.f);
    this->color_out[0] = vbo->m_color * tex.sample<float>(vbo->m_uv);
  }
};
template struct CParticleSwooshShadersTexPipeline<true, true, false>;
template struct CParticleSwooshShadersTexPipeline<false, true, true>;

template <bool Additive, bool AlphaWrite, bool ZWrite>
struct CParticleSwooshShadersNoTexPipeline
: pipeline<topology<hsh::TriangleStrip>,
           std::conditional_t<Additive, AdditiveAttachment<AlphaWrite>, BlendAttachment<AlphaWrite>>,
           depth_compare<hsh::LEqual>, depth_write<ZWrite>> {
  CParticleSwooshShadersNoTexPipeline(hsh::vertex_buffer<CParticleSwooshShaders::Vert> vbo,
                                      hsh::uniform_buffer<CParticleSwooshShaders::Uniform> uniBuf) {
    this->position = uniBuf->m_xf * hsh::float4(vbo->m_pos, 1.f);
    this->color_out[0] = vbo->m_color;
  }
};
template struct CParticleSwooshShadersNoTexPipeline<true, true, false>;
template struct CParticleSwooshShadersNoTexPipeline<false, true, true>;

CParticleSwooshShaders::EShaderClass CParticleSwooshShaders::GetShaderClass(CParticleSwoosh& gen) {
  return gen.GetDesc()->x3c_TEXR ? EShaderClass::Tex : EShaderClass::NoTex;
}

void CParticleSwooshShaders::BuildShaderDataBinding(CParticleSwoosh& gen) {
  auto* desc = gen.GetDesc();
  switch (GetShaderClass(gen)) {
  case EShaderClass::Tex: {
    hsh::texture2d tex = desc->x3c_TEXR->GetValueTexture(0)->GetBooTexture();
    gen.m_dataBind[0].hsh_tex_noalphawrite_bind(
        CParticleSwooshShadersTexPipeline<desc->x44_31_AALP, false, desc->x45_24_ZBUF>(gen.m_vertBuf.get(),
                                                                                       gen.m_uniformBuf.get(), tex));
    gen.m_dataBind[1].hsh_tex_alphawrite_bind(
        CParticleSwooshShadersTexPipeline<desc->x44_31_AALP, true, desc->x45_24_ZBUF>(gen.m_vertBuf.get(),
                                                                                      gen.m_uniformBuf.get(), tex));
    break;
  }
  case EShaderClass::NoTex: {
    gen.m_dataBind[0].hsh_notex_noalphawrite_bind(
        CParticleSwooshShadersNoTexPipeline<desc->x44_31_AALP, false, desc->x45_24_ZBUF>(gen.m_vertBuf.get(),
                                                                                         gen.m_uniformBuf.get()));
    gen.m_dataBind[1].hsh_notex_alphawrite_bind(
        CParticleSwooshShadersNoTexPipeline<desc->x44_31_AALP, true, desc->x45_24_ZBUF>(gen.m_vertBuf.get(),
                                                                                        gen.m_uniformBuf.get()));
    break;
  }
  }
}

} // namespace metaforce
