#include "Runtime/Graphics/Shaders/CLineRendererShaders.hpp"

#include <utility>

#include "Runtime/Graphics/CLineRenderer.hpp"

#include <hecl/Pipeline.hpp>

namespace urde {

std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CLineRendererShaders::m_texAlpha;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CLineRendererShaders::m_texAdditive;

std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CLineRendererShaders::m_noTexAlpha;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CLineRendererShaders::m_noTexAdditive;

std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CLineRendererShaders::m_texAlphaZ;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CLineRendererShaders::m_texAdditiveZ;

std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CLineRendererShaders::m_noTexAlphaZ;
std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CLineRendererShaders::m_noTexAdditiveZ;

std::array<boo::ObjToken<boo::IShaderPipeline>, 2> CLineRendererShaders::m_noTexAlphaZGEqual;

void CLineRendererShaders::Initialize() {
  CGraphics::CommitResources([](boo::IGraphicsDataFactory::Context& ctx) {
    m_texAlpha = {hecl::conv->convert(ctx, Shader_CLineRendererShaderTexAlpha{}),
                  hecl::conv->convert(ctx, Shader_CLineRendererShaderTexAlphaAWrite{})};
    m_texAdditive = {hecl::conv->convert(ctx, Shader_CLineRendererShaderTexAdditive{}),
                     hecl::conv->convert(ctx, Shader_CLineRendererShaderTexAdditiveAWrite{})};
    m_noTexAlpha = {hecl::conv->convert(ctx, Shader_CLineRendererShaderNoTexAlpha{}),
                    hecl::conv->convert(ctx, Shader_CLineRendererShaderNoTexAlphaAWrite{})};
    m_noTexAdditive = {hecl::conv->convert(ctx, Shader_CLineRendererShaderNoTexAdditive{}),
                       hecl::conv->convert(ctx, Shader_CLineRendererShaderNoTexAdditiveAWrite{})};
    m_texAlphaZ = {hecl::conv->convert(ctx, Shader_CLineRendererShaderTexAlphaZ{}),
                   hecl::conv->convert(ctx, Shader_CLineRendererShaderTexAlphaZAWrite{})};
    m_texAdditiveZ = {hecl::conv->convert(ctx, Shader_CLineRendererShaderTexAdditiveZ{}),
                      hecl::conv->convert(ctx, Shader_CLineRendererShaderTexAdditiveZAWrite{})};
    m_noTexAlphaZ = {hecl::conv->convert(ctx, Shader_CLineRendererShaderNoTexAlphaZ{}),
                     hecl::conv->convert(ctx, Shader_CLineRendererShaderNoTexAlphaZAWrite{})};
    m_noTexAdditiveZ = {hecl::conv->convert(ctx, Shader_CLineRendererShaderNoTexAdditiveZ{}),
                        hecl::conv->convert(ctx, Shader_CLineRendererShaderNoTexAdditiveZAWrite{})};
    m_noTexAlphaZGEqual = {hecl::conv->convert(ctx, Shader_CLineRendererShaderNoTexAlphaZGEqual{}),
                           hecl::conv->convert(ctx, Shader_CLineRendererShaderNoTexAlphaZGEqualAWrite{})};
    return true;
  } BooTrace);
}

void CLineRendererShaders::Shutdown() {
  for (auto& s : m_texAlpha) s.reset();
  for (auto& s : m_texAdditive) s.reset();
  for (auto& s : m_noTexAlpha) s.reset();
  for (auto& s : m_noTexAdditive) s.reset();
  for (auto& s : m_texAlphaZ) s.reset();
  for (auto& s : m_texAdditiveZ) s.reset();
  for (auto& s : m_noTexAlphaZ) s.reset();
  for (auto& s : m_noTexAdditiveZ) s.reset();
  for (auto& s : m_noTexAlphaZGEqual) s.reset();
}

void CLineRendererShaders::BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CLineRenderer& renderer,
                                                  const boo::ObjToken<boo::ITexture>& texture, bool additive,
                                                  bool zTest, bool zGEqual) {
  std::array<boo::ObjToken<boo::IShaderPipeline>, 2>* pipeline = nullptr;

  if (zGEqual) {
    pipeline = &m_noTexAlphaZGEqual;
  } else if (zTest) {
    if (texture) {
      if (additive)
        pipeline = &m_texAdditiveZ;
      else
        pipeline = &m_texAlphaZ;
    } else {
      if (additive)
        pipeline = &m_noTexAdditiveZ;
      else
        pipeline = &m_noTexAlphaZ;
    }
  } else {
    if (texture) {
      if (additive)
        pipeline = &m_texAdditive;
      else
        pipeline = &m_texAlpha;
    } else {
      if (additive)
        pipeline = &m_noTexAdditive;
      else
        pipeline = &m_noTexAlpha;
    }
  }

  int texCount = 0;
  boo::ObjToken<boo::ITexture> textures[1];

  std::pair<boo::ObjToken<boo::IGraphicsBufferD>, hecl::VertexBufferPool<CLineRenderer::SDrawVertTex>::IndexTp>
      vbufInfo;
  std::pair<boo::ObjToken<boo::IGraphicsBufferD>, hecl::UniformBufferPool<CLineRenderer::SDrawUniform>::IndexTp>
      ubufInfo = renderer.m_uniformBuf.getBufferInfo();
  if (texture) {
    vbufInfo = renderer.m_vertBufTex.getBufferInfo();
    textures[0] = texture;
    texCount = 1;
  } else {
    vbufInfo = renderer.m_vertBufNoTex.getBufferInfo();
  }

  boo::ObjToken<boo::IGraphicsBuffer> uniforms[] = {ubufInfo.first.get()};
  boo::PipelineStage stages[] = {boo::PipelineStage::Fragment};
  size_t ubufOffs[] = {size_t(ubufInfo.second)};
  size_t ubufSizes[] = {sizeof(CLineRenderer::SDrawUniform)};

  for (int i = 0; i < 2; ++i)
    renderer.m_shaderBind[i] =
      ctx.newShaderDataBinding((*pipeline)[i], vbufInfo.first.get(), nullptr, nullptr, 1, uniforms, stages, ubufOffs,
                               ubufSizes, texCount, textures, nullptr, nullptr, vbufInfo.second);
}

} // namespace urde
