#include "Runtime/Graphics/Shaders/CElementGenShaders.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Particle/CElementGen.hpp"

enum class BlendMode {
  Regular,
  Additive,
  Subtract,
};

#include "CElementGenShaders.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

template <BlendMode Mode, bool AlphaWrite>
struct BlendModeAttachmentExt {
  using type = BlendAttachment<AlphaWrite>;
};
template <bool AlphaWrite>
struct BlendModeAttachmentExt<BlendMode::Additive, AlphaWrite> {
  using type = AdditiveAttachment<AlphaWrite>;
};
template <bool AlphaWrite>
struct BlendModeAttachmentExt<BlendMode::Subtract, AlphaWrite> {
  using type = SubtractAttachment<AlphaWrite>;
};
template <BlendMode Mode, bool AlphaWrite>
using BlendModeAttachment = typename BlendModeAttachmentExt<Mode, AlphaWrite>::type;

template <BlendMode Mode, bool AlphaWrite, bool ZTest, bool ZWrite, bool RedToAlpha>
struct CElementGenShadersTexPipeline : pipeline<topology<hsh::TriangleStrip>, BlendModeAttachment<Mode, AlphaWrite>,
                                                depth_compare<ZTest ? hsh::LEqual : hsh::Always>, depth_write<ZWrite>> {
  CElementGenShadersTexPipeline(hsh::vertex_buffer<SParticleInstanceTex> vbo HSH_VAR_INSTANCE,
                                hsh::uniform_buffer<SParticleUniforms> uniBuf, hsh::texture2d tex) {
    this->position = uniBuf->mvp * vbo->pos[this->vertex_id];
    this->color_out[0] = vbo->color * uniBuf->moduColor * tex.sample<float>(vbo->uvs[this->vertex_id]);
    if constexpr (RedToAlpha) {
      this->color_out[0].w = this->color_out[0].x;
    }
  }
};
template struct CElementGenShadersTexPipeline<BlendMode::Additive, false, true, false, false>;
template struct CElementGenShadersTexPipeline<BlendMode::Regular, false, true, false, false>;

template <BlendMode Mode, bool AlphaWrite, bool ZTest, bool ZWrite, bool ColoredIndirect>
struct CElementGenShadersIndTexPipeline
: pipeline<topology<hsh::TriangleStrip>, BlendModeAttachment<Mode, AlphaWrite>,
           depth_compare<ZTest ? hsh::LEqual : hsh::Always>, depth_write<ZWrite>> {
  CElementGenShadersIndTexPipeline(hsh::vertex_buffer<SParticleInstanceIndTex> vbo HSH_VAR_INSTANCE,
                                   hsh::uniform_buffer<SParticleUniforms> uniBuf, hsh::texture2d texrTex,
                                   hsh::texture2d tindTex, hsh::render_texture2d sceneTex) {
    this->position = uniBuf->mvp * vbo->pos[this->vertex_id];
    hsh::float4 texrTexel = texrTex.sample<float>(vbo->texrTindUVs[this->vertex_id].xy());
    hsh::float2 tindTexel = tindTex.sample<float>(vbo->texrTindUVs[this->vertex_id].zw()).xy();
    hsh::float4 sceneTexel = sceneTex.sample<float>(hsh::lerp(vbo->sceneUVs.xy(), vbo->sceneUVs.zw(), tindTexel));
    if constexpr (ColoredIndirect) {
      this->color_out[0] = vbo->color * hsh::float4(sceneTexel.xyz(), 1.f) * texrTexel;
    } else {
      this->color_out[0] = vbo->color * hsh::float4(sceneTexel.xyz(), 1.f) + texrTexel;
      this->color_out[0].w *= texrTexel.x;
    }
  }
};
template struct CElementGenShadersIndTexPipeline<BlendMode::Regular, false, true, false, false>;

template <BlendMode Mode, bool AlphaWrite, bool ZTest, bool ZWrite>
struct CElementGenShadersNoTexPipeline
: pipeline<topology<hsh::TriangleStrip>, BlendModeAttachment<Mode, AlphaWrite>,
           depth_compare<ZTest ? hsh::LEqual : hsh::Always>, depth_write<ZWrite>> {
  CElementGenShadersNoTexPipeline(hsh::vertex_buffer<SParticleInstanceNoTex> vbo HSH_VAR_INSTANCE,
                                  hsh::uniform_buffer<SParticleUniforms> uniBuf) {
    this->position = uniBuf->mvp * vbo->pos[this->vertex_id];
    this->color_out[0] = vbo->color * uniBuf->moduColor;
  }
};
template struct CElementGenShadersNoTexPipeline<BlendMode::Regular, false, true, false>;
template struct CElementGenShadersNoTexPipeline<BlendMode::Additive, false, true, false>;

CElementGenShaders::EShaderClass CElementGenShaders::GetShaderClass(CElementGen& gen) {
  const auto* desc = gen.x1c_genDesc.GetObj();
  if (desc->x54_x40_TEXR) {
    if (desc->x58_x44_TIND) {
      return EShaderClass::IndTex;
    }
    return EShaderClass::Tex;
  }
  return EShaderClass::NoTex;
}

void CElementGenShaders::BuildShaderDataBinding(CElementGen& gen) {
  const auto& desc = gen.x1c_genDesc;
  BlendMode mode = BlendMode::Regular;
  BlendMode pmusMode = BlendMode::Regular;
  if (CElementGen::g_subtractBlend) {
    mode = BlendMode::Subtract;
    pmusMode = BlendMode::Subtract;
  } else {
    if (gen.x26c_26_AAPH) {
      mode = BlendMode::Additive;
    }
    if (desc->x44_31_x31_25_PMAB) {
      pmusMode = BlendMode::Additive;
    }
  }
  switch (GetShaderClass(gen)) {
  case EShaderClass::Tex: {
    hsh::texture2d tex = desc->x54_x40_TEXR->GetValueTexture(0)->GetBooTexture();
    gen.m_binding.hsh_tex_bind(
        CElementGenShadersTexPipeline<mode, g_Renderer->IsThermalVisorHotPass(), gen.x26c_28_zTest, gen.x26c_27_ZBUF,
                                      CElementGen::sMoveRedToAlphaBuffer>(gen.m_instBuf.get(), gen.m_uniformBuf.get(),
                                                                          tex));
    if (gen.x1c_genDesc->x45_24_x31_26_PMUS)
      gen.m_bindingPmus.hsh_tex_pmus_bind(
          CElementGenShadersTexPipeline<pmusMode, g_Renderer->IsThermalVisorHotPass(), gen.x26c_28_zTest, gen.x26c_27_ZBUF,
                                        CElementGen::sMoveRedToAlphaBuffer>(gen.m_instBufPmus.get(),
                                                                            gen.m_uniformBufPmus.get(), tex));
    break;
  }
  case EShaderClass::IndTex: {
    hsh::texture2d texrTex = desc->x54_x40_TEXR->GetValueTexture(0)->GetBooTexture();
    hsh::texture2d tindTex = desc->x58_x44_TIND->GetValueTexture(0)->GetBooTexture();
    hsh::render_texture2d sceneTex = CGraphics::g_SpareTexture.get_color(0);
    gen.m_binding.hsh_indtex_bind(
        CElementGenShadersIndTexPipeline<mode, g_Renderer->IsThermalVisorHotPass(), gen.x26c_28_zTest, gen.x26c_27_ZBUF,
                                         desc->x45_30_x32_24_CIND>(gen.m_instBuf.get(), gen.m_uniformBuf.get(), texrTex,
                                                                   tindTex, sceneTex));
    if (gen.x1c_genDesc->x45_24_x31_26_PMUS)
      gen.m_bindingPmus.hsh_indtex_pmus_bind(
          CElementGenShadersIndTexPipeline<pmusMode, g_Renderer->IsThermalVisorHotPass(), gen.x26c_28_zTest,
                                           gen.x26c_27_ZBUF, desc->x45_30_x32_24_CIND>(
              gen.m_instBufPmus.get(), gen.m_uniformBufPmus.get(), texrTex, tindTex, sceneTex));
    break;
  }
  case EShaderClass::NoTex: {
    gen.m_binding.hsh_notex_bind(
        CElementGenShadersNoTexPipeline<mode, g_Renderer->IsThermalVisorHotPass(), gen.x26c_28_zTest, gen.x26c_27_ZBUF>(
            gen.m_instBuf.get(), gen.m_uniformBuf.get()));
    if (gen.x1c_genDesc->x45_24_x31_26_PMUS)
      gen.m_bindingPmus.hsh_notex_pmus_bind(
          CElementGenShadersNoTexPipeline<pmusMode, g_Renderer->IsThermalVisorHotPass(), gen.x26c_28_zTest,
                                          gen.x26c_27_ZBUF>(gen.m_instBufPmus.get(), gen.m_uniformBufPmus.get()));
    break;
  }
  }
}

} // namespace urde
