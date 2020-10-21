#include "Runtime/Graphics/Shaders/CEnvFxShaders.hpp"

#include "Runtime/World/CEnvFxManager.hpp"

#include "CEnvFxShaders.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

template <bool Blend>
struct CEnvFxShadersPipeline
// FIXME replace MultiplyAttachment with ERglBlendFactor One:One equivalent
: pipeline<topology<hsh::TriangleStrip>, std::conditional_t<Blend, BlendAttachment<false>, MultiplyAttachment<false>>,
           depth_compare<hsh::LEqual>, depth_write<false>> {
  CEnvFxShadersPipeline(hsh::vertex_buffer<CEnvFxShaders::Instance> vbo HSH_VAR_INSTANCE,
                        hsh::uniform_buffer<CEnvFxShaders::Uniform> envFxUniBuf,
                        hsh::uniform_buffer<CGraphics::CFogState> fogUniBuf, hsh::texture2d texFlake,
                        hsh::texture2d texEnv) {
    hsh::float4 posIn = hsh::float4(vbo->positions[this->vertex_id], 1.f);
    hsh::float4 flakeTexel = texFlake.sample<float>(vbo->uvs[this->vertex_id]);
    hsh::float4 envTexel = texEnv.sample<float>(
        (envFxUniBuf->envMtx * posIn).xy(),
        // FIXME hsh bug: this appears to be completely ignored
        hsh::sampler{hsh::Linear, hsh::Linear, hsh::Linear, hsh::ClampToEdge, hsh::ClampToEdge, hsh::ClampToEdge});
    hsh::float4 color = vbo->color * envFxUniBuf->moduColor * flakeTexel * envTexel;
    this->position = envFxUniBuf->proj * (envFxUniBuf->mv * posIn);
    FOG_SHADER_UNIFORM(fogUniBuf)
    FOG_OUT_UNIFORM(this->color_out[0], fogUniBuf, color)
  }
};
template struct CEnvFxShadersPipeline<true>;
template struct CEnvFxShadersPipeline<false>;

void CEnvFxShaders::BuildShaderDataBinding(CEnvFxManager& fxMgr, CEnvFxManagerGrid& grid) {
  hsh::texture2d texFlake = fxMgr.xb74_txtrSnowFlake->GetBooTexture();
  hsh::texture2d texEnv = fxMgr.x40_txtrEnvGradient->GetBooTexture();
  hsh::vertex_buffer<Instance> vboBuf = grid.m_instBuf.get();
  hsh::uniform_buffer<Uniform> envFxUniBuf = grid.m_uniformBuf.get();
  hsh::uniform_buffer<CGraphics::CFogState> fogUniBuf = fxMgr.m_fogUniformBuf.get();
  // FIXME hsh bug: can't bind all constant values
  bool isUnderwater = false;
  grid.m_snowBinding.hsh_snow_bind(
      CEnvFxShadersPipeline<isUnderwater>(vboBuf, envFxUniBuf, fogUniBuf, texFlake, texEnv));
  isUnderwater = true;
  grid.m_underwaterBinding.hsh_underwater_bind(
      CEnvFxShadersPipeline<isUnderwater>(vboBuf, envFxUniBuf, fogUniBuf, texFlake, texEnv));
}

} // namespace urde
