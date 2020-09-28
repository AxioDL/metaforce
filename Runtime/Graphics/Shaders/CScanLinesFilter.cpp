#include "Runtime/Graphics/Shaders/CScanLinesFilter.hpp"

#include <array>

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

namespace urde {

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
    const std::array<boo::ObjToken<boo::IGraphicsBuffer>, 1> bufs{m_uniBuf.get()};
    constexpr std::array<boo::PipelineStage, 1> stages{boo::PipelineStage::Vertex};

    m_dataBind = ctx.newShaderDataBinding(SelectPipeline(type), vbo, nullptr, nullptr, bufs.size(), bufs.data(),
                                          stages.data(), nullptr, nullptr, 0, nullptr, nullptr, nullptr);
    return true;
  } BooTrace);
}

void CScanLinesFilter::draw(const zeus::CColor& color) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CScanLinesFilter::draw", zeus::skMagenta);

  m_uniform.color = color;
  m_uniBuf->load(&m_uniform, sizeof(Uniform));

  CGraphics::SetShaderDataBinding(m_dataBind);
  CGraphics::DrawArray(0, 670);
}

} // namespace urde
