#include "CScanLinesFilter.hpp"
#include "hecl/Pipeline.hpp"
#include "Graphics/CGraphics.hpp"
#include "GameGlobalObjects.hpp"
#include "Graphics/CBooRenderer.hpp"

namespace urde {

static boo::ObjToken<boo::IShaderPipeline> s_AlphaPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AddPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_MultPipeline;

void CScanLinesFilter::Initialize() {
  s_AlphaPipeline = hecl::conv->convert(Shader_CScanLinesFilterAlpha{});
  s_AddPipeline = hecl::conv->convert(Shader_CScanLinesFilterAdd{});
  s_MultPipeline = hecl::conv->convert(Shader_CScanLinesFilterMult{});
}

void CScanLinesFilter::Shutdown() {
  s_AlphaPipeline.reset();
  s_AddPipeline.reset();
  s_MultPipeline.reset();
}

static boo::ObjToken<boo::IShaderPipeline> SelectPipeline(EFilterType type) {
  switch (type) {
  case EFilterType::Blend:
    return s_AlphaPipeline;
  case EFilterType::Add:
    return s_AddPipeline;
  case EFilterType::Multiply:
    return s_MultPipeline;
  default:
    return {};
  }
}

CScanLinesFilter::CScanLinesFilter(EFilterType type, bool even) : m_even(even) {
  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
    m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
    boo::ObjToken<boo::IGraphicsBuffer> vbo =
        m_even ? g_Renderer->GetScanLinesEvenVBO().get() : g_Renderer->GetScanLinesOddVBO().get();
    boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {m_uniBuf.get()};
    boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
    m_dataBind = ctx.newShaderDataBinding(SelectPipeline(type), vbo, nullptr, nullptr, 1, bufs, stages, nullptr,
                                          nullptr, 0, nullptr, nullptr, nullptr);
    return true;
  } BooTrace);
}

void CScanLinesFilter::draw(const zeus::CColor& color) {
  m_uniform.color = color;
  m_uniBuf->load(&m_uniform, sizeof(Uniform));

  CGraphics::SetShaderDataBinding(m_dataBind);
  CGraphics::DrawArray(0, 670);
}

} // namespace urde
