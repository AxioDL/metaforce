#include "Runtime/Graphics/Shaders/CScanLinesFilter.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

#include "CScanLinesFilter.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

template <EFilterType Type>
struct CScanLinesFilterPipeline : FilterPipeline<Type> {
  CScanLinesFilterPipeline(hsh::vertex_buffer<CBooRenderer::ScanlinesVert> vbo,
                           hsh::uniform_buffer<CScanLinesFilter::Uniform> ubo) {
    this->position = hsh::float4(vbo->pos, 1.f);
    this->color_out[0] = ubo->color;
  }
};
template struct CScanLinesFilterPipeline<EFilterType::Blend>;
template struct CScanLinesFilterPipeline<EFilterType::Add>;
// TODO old shader sets #overwritealpha true
// but isn't implemented in FilterPipeline
template struct CScanLinesFilterPipeline<EFilterType::Multiply>;

CScanLinesFilter::CScanLinesFilter(EFilterType type, bool even) : m_even(even) {
  m_uniBuf = hsh::create_dynamic_uniform_buffer<Uniform>();
  m_dataBind.hsh_bind(CScanLinesFilterPipeline<type>(
      m_even ? g_Renderer->GetScanLinesEvenVBO() : g_Renderer->GetScanLinesOddVBO(), m_uniBuf.get()));
}

void CScanLinesFilter::draw(const zeus::CColor& color) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CScanLinesFilter::draw", zeus::skMagenta);

  m_uniform.color = color;
#if !HSH_PROFILE_MODE
  m_uniBuf.load(m_uniform);
#endif

  m_dataBind.draw(0, 670);
}

} // namespace urde
