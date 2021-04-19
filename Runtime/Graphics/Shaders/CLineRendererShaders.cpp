#include "Runtime/Graphics/Shaders/CLineRendererShaders.hpp"
#include "Runtime/Graphics/CLineRenderer.hpp"
#include "CLineRendererShaders.cpp.hshhead"

namespace metaforce {
using namespace hsh::pipeline;

template <bool Additive, bool AlphaWrite, hsh::Compare ZComp>
struct CLineRendererTexPipeline
: pipeline<topology<hsh::TriangleStrip>,
           std::conditional_t<Additive, AdditiveAttachment<AlphaWrite>, BlendAttachment<AlphaWrite>>,
           depth_compare<ZComp>> {
  CLineRendererTexPipeline(hsh::vertex_buffer<CLineRenderer::SDrawVertTex> vbo,
                           hsh::uniform_buffer<CLineRenderer::SDrawUniform> uniBuf HSH_VAR_STAGE(fragment),
                           hsh::texture2d tex) {
    this->position = vbo->pos;
    hsh::float4 colorIn = vbo->color * uniBuf->moduColor * tex.sample<float>(vbo->uv);
    FOG_SHADER(uniBuf->fog)
    FOG_OUT(this->color_out[0], uniBuf->fog, colorIn)
  }
};
template struct CLineRendererTexPipeline<false, false, hsh::Always>;
template struct CLineRendererTexPipeline<false, false, hsh::LEqual>;
template struct CLineRendererTexPipeline<false, false, hsh::Greater>;
template struct CLineRendererTexPipeline<false, true, hsh::Always>;
template struct CLineRendererTexPipeline<false, true, hsh::LEqual>;
template struct CLineRendererTexPipeline<false, true, hsh::Greater>;
template struct CLineRendererTexPipeline<true, false, hsh::Always>;
template struct CLineRendererTexPipeline<true, false, hsh::LEqual>;
template struct CLineRendererTexPipeline<true, false, hsh::Greater>;
template struct CLineRendererTexPipeline<true, true, hsh::Always>;
template struct CLineRendererTexPipeline<true, true, hsh::LEqual>;
template struct CLineRendererTexPipeline<true, true, hsh::Greater>;

template <bool Additive, bool AlphaWrite, hsh::Compare ZComp>
struct CLineRendererNoTexPipeline
: pipeline<topology<hsh::TriangleStrip>,
           std::conditional_t<Additive, AdditiveAttachment<AlphaWrite>, BlendAttachment<AlphaWrite>>,
           depth_compare<ZComp>> {
  CLineRendererNoTexPipeline(hsh::vertex_buffer<CLineRenderer::SDrawVertNoTex> vbo,
                             hsh::uniform_buffer<CLineRenderer::SDrawUniform> uniBuf HSH_VAR_STAGE(fragment)) {
    this->position = vbo->pos;
    hsh::float4 colorIn = vbo->color * uniBuf->moduColor;
    FOG_SHADER(uniBuf->fog)
    FOG_OUT(this->color_out[0], uniBuf->fog, colorIn)
  }
};
template struct CLineRendererNoTexPipeline<false, false, hsh::Always>;
template struct CLineRendererNoTexPipeline<false, false, hsh::LEqual>;
template struct CLineRendererNoTexPipeline<false, false, hsh::Greater>;
template struct CLineRendererNoTexPipeline<false, false, hsh::GEqual>;
template struct CLineRendererNoTexPipeline<false, true, hsh::Always>;
template struct CLineRendererNoTexPipeline<false, true, hsh::LEqual>;
template struct CLineRendererNoTexPipeline<false, true, hsh::Greater>;
template struct CLineRendererNoTexPipeline<true, false, hsh::Always>;
template struct CLineRendererNoTexPipeline<true, false, hsh::LEqual>;
template struct CLineRendererNoTexPipeline<true, false, hsh::Greater>;
template struct CLineRendererNoTexPipeline<true, true, hsh::Always>;
template struct CLineRendererNoTexPipeline<true, true, hsh::LEqual>;
template struct CLineRendererNoTexPipeline<true, true, hsh::Greater>;

void CLineRendererShaders::BindShader(CLineRenderer& renderer, hsh::texture2d texture, bool additive,
                                      hsh::Compare zcomp) {
  if (texture) {
    renderer.m_shaderBind[0].hsh_tex_bind(CLineRendererTexPipeline<additive, false, zcomp>(
        renderer.m_vertBuf.get(), renderer.m_uniformBuf.get(), texture));
    renderer.m_shaderBind[1].hsh_tex_bind_awrite(CLineRendererTexPipeline<additive, true, zcomp>(
        renderer.m_vertBuf.get(), renderer.m_uniformBuf.get(), texture));
  } else {
    renderer.m_shaderBind[0].hsh_notex_bind(
        CLineRendererNoTexPipeline<additive, false, zcomp>(renderer.m_vertBuf.get(), renderer.m_uniformBuf.get()));
    renderer.m_shaderBind[1].hsh_notex_bind_awrite(
        CLineRendererNoTexPipeline<additive, true, zcomp>(renderer.m_vertBuf.get(), renderer.m_uniformBuf.get()));
  }
}

} // namespace metaforce
