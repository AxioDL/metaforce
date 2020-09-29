#include "Runtime/Graphics/Shaders/CDecalShaders.hpp"

#include <array>

#include "Runtime/Particle/CDecal.hpp"

#include "CDecalShaders.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

template <bool Additive, bool RedToAlpha>
struct CDecalShaderTexPipeline
: pipeline<std::conditional_t<Additive, std::conditional_t<RedToAlpha, MultiplyAttachment<>, AdditiveAttachment<>>,
                              BlendAttachment<>>,
           depth_compare<hsh::LEqual>, depth_write<false>> {
  CDecalShaderTexPipeline(hsh::vertex_buffer<SParticleInstanceTex> vbo, hsh::uniform_buffer<SParticleUniforms> uniBuf,
                          hsh::texture2d tex) {
    this->position = uniBuf->mvp * vbo->pos[this->vertex_id];
    this->color_out[0] = vbo->color * uniBuf->moduColor * tex.sample<float>(vbo->uvs[this->vertex_id]);
    if constexpr (RedToAlpha) {
      this->color_out[0].w = this->color_out[0].x;
    }
  }
};
template struct CDecalShaderTexPipeline<true, true>;
template struct CDecalShaderTexPipeline<true, false>;
template struct CDecalShaderTexPipeline<false, false>;

template <bool Additive>
struct CDecalShaderNoTexPipeline : pipeline<std::conditional_t<Additive, AdditiveAttachment<>, BlendAttachment<>>,
                                            depth_compare<hsh::LEqual>, depth_write<!Additive>> {
  CDecalShaderNoTexPipeline(hsh::vertex_buffer<SParticleInstanceTex> vbo,
                            hsh::uniform_buffer<SParticleUniforms> uniBuf) {
    this->position = uniBuf->mvp * vbo->pos[this->vertex_id];
    this->color_out[0] = vbo->color * uniBuf->moduColor;
  }
};
template struct CDecalShaderNoTexPipeline<true>;
template struct CDecalShaderNoTexPipeline<false>;

hsh::binding& CDecalShaders::BuildShaderDataBinding(CQuadDecal& decal, hsh::texture2d tex) {
  bool additive = decal.m_desc->x18_ADD;
  if (decal.m_desc->x14_TEX) {
    bool redToAlpha = additive && CDecal::GetMoveRedToAlphaBuffer();
    m_dataBind.hsh_tex_bind(
        CDecalShaderTexPipeline<additive, redToAlpha>(decal.m_instBuf.get(), decal.m_uniformBuf.get(), tex));
  } else {
    m_dataBind.hsh_notex_bind(CDecalShaderNoTexPipeline<additive>(decal.m_instBuf.get(), decal.m_uniformBuf.get()));
  }
  return m_dataBind;
}

} // namespace urde
