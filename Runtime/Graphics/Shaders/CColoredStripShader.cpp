#include "CColoredStripShader.hpp"
#include "Graphics/CGraphics.hpp"
#include "hecl/Pipeline.hpp"
#include "GameGlobalObjects.hpp"
#include "Graphics/CBooRenderer.hpp"

namespace urde {

static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AdditivePipeline;
static boo::ObjToken<boo::IShaderPipeline> s_FullAdditivePipeline;

void CColoredStripShader::Initialize() {
  s_Pipeline = hecl::conv->convert(Shader_CColoredStripShader{});
  s_AdditivePipeline = hecl::conv->convert(Shader_CColoredStripShaderAdditive{});
  s_FullAdditivePipeline = hecl::conv->convert(Shader_CColoredStripShaderFullAdditive{});
}

void CColoredStripShader::Shutdown() {
  s_Pipeline.reset();
  s_AdditivePipeline.reset();
  s_FullAdditivePipeline.reset();
}

static const boo::ObjToken<boo::IShaderPipeline>& SelectPipeline(CColoredStripShader::Mode mode) {
  switch (mode) {
  case CColoredStripShader::Mode::Alpha:
  default:
    return s_Pipeline;
  case CColoredStripShader::Mode::Additive:
    return s_AdditivePipeline;
  case CColoredStripShader::Mode::FullAdditive:
    return s_FullAdditivePipeline;
  }
}

void CColoredStripShader::BuildResources(boo::IGraphicsDataFactory::Context& ctx, size_t maxVerts, Mode mode,
                                         boo::ObjToken<boo::ITexture> tex) {
  m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, sizeof(Vert), maxVerts);
  m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
  boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {m_uniBuf.get()};
  boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
  boo::ObjToken<boo::ITexture> texs[1];
  if (tex)
    texs[0] = tex;
  else
    texs[0] = g_Renderer->GetWhiteTexture();
  m_dataBind = ctx.newShaderDataBinding(SelectPipeline(mode), m_vbo.get(), nullptr, nullptr, 1, bufs, stages, nullptr,
                                        nullptr, 1, texs, nullptr, nullptr);
}

CColoredStripShader::CColoredStripShader(size_t maxVerts, Mode mode, boo::ObjToken<boo::ITexture> tex) {
  CGraphics::CommitResources([this, maxVerts, mode, tex](boo::IGraphicsDataFactory::Context& ctx) {
    BuildResources(ctx, maxVerts, mode, tex);
    return true;
  } BooTrace);
}

CColoredStripShader::CColoredStripShader(boo::IGraphicsDataFactory::Context& ctx, size_t maxVerts, Mode mode,
                                         boo::ObjToken<boo::ITexture> tex) {
  BuildResources(ctx, maxVerts, mode, tex);
}

void CColoredStripShader::draw(const zeus::CColor& color, size_t numVerts, const Vert* verts) {
  m_vbo->load(verts, sizeof(Vert) * numVerts);

  m_uniform.m_matrix = CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f();
  m_uniform.m_color = color;
  m_uniBuf->load(&m_uniform, sizeof(m_uniform));

  CGraphics::SetShaderDataBinding(m_dataBind);
  CGraphics::DrawArray(0, numVerts);
}

}
