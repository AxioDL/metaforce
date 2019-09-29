#include "Runtime/Graphics/Shaders/CColoredStripShader.hpp"

#include <array>

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

#include <hecl/Pipeline.hpp>

namespace urde {

static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AdditivePipeline;
static boo::ObjToken<boo::IShaderPipeline> s_FullAdditivePipeline;
static boo::ObjToken<boo::IShaderPipeline> s_SubtractivePipeline;

void CColoredStripShader::Initialize() {
  s_Pipeline = hecl::conv->convert(Shader_CColoredStripShader{});
  s_AdditivePipeline = hecl::conv->convert(Shader_CColoredStripShaderAdditive{});
  s_FullAdditivePipeline = hecl::conv->convert(Shader_CColoredStripShaderFullAdditive{});
  s_SubtractivePipeline = hecl::conv->convert(Shader_CColoredStripShaderSubtractive{});
}

void CColoredStripShader::Shutdown() {
  s_Pipeline.reset();
  s_AdditivePipeline.reset();
  s_FullAdditivePipeline.reset();
  s_SubtractivePipeline.reset();
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
  case CColoredStripShader::Mode::Subtractive:
    return s_SubtractivePipeline;
  }
}

void CColoredStripShader::BuildResources(boo::IGraphicsDataFactory::Context& ctx, size_t maxVerts, Mode mode,
                                         boo::ObjToken<boo::ITexture> tex) {
  m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, sizeof(Vert), maxVerts);
  m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);

  const std::array<boo::ObjToken<boo::IGraphicsBuffer>, 1> bufs{m_uniBuf.get()};
  constexpr std::array<boo::PipelineStage, 1> stages{boo::PipelineStage::Vertex};
  std::array<boo::ObjToken<boo::ITexture>, 1> texs;
  if (tex) {
    texs[0] = tex;
  } else {
    texs[0] = g_Renderer->GetWhiteTexture();
  }

  m_dataBind = ctx.newShaderDataBinding(SelectPipeline(mode), m_vbo.get(), nullptr, nullptr, bufs.size(), bufs.data(),
                                        stages.data(), nullptr, nullptr, texs.size(), texs.data(), nullptr, nullptr);
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
  SCOPED_GRAPHICS_DEBUG_GROUP("CColoredStripShader::draw", zeus::skMagenta);

  m_vbo->load(verts, sizeof(Vert) * numVerts);

  m_uniform.m_matrix = CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f();
  m_uniform.m_color = color;
  m_uniBuf->load(&m_uniform, sizeof(m_uniform));

  CGraphics::SetShaderDataBinding(m_dataBind);
  CGraphics::DrawArray(0, numVerts);
}

}
