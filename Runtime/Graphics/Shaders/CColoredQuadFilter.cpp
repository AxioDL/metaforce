#include "Runtime/Graphics/Shaders/CColoredQuadFilter.hpp"

#include <array>

#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

#include "CColoredQuadFilter.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

template <EFilterType Type>
struct CColoredQuadFilterPipeline : FilterPipeline<Type> {
  CColoredQuadFilterPipeline(hsh::vertex_buffer<CColoredQuadFilter::Vert> vbo,
                             hsh::uniform_buffer<CColoredQuadFilter::Uniform> uniBuf) {
    this->position = uniBuf->m_matrix * hsh::float4(vbo->m_pos, 1.f);
    this->color_out[0] = uniBuf->m_color;
  }
};
template struct CColoredQuadFilterPipeline<EFilterType::Add>;
template struct CColoredQuadFilterPipeline<EFilterType::Blend>;
template struct CColoredQuadFilterPipeline<EFilterType::Multiply>;

CColoredQuadFilter::CColoredQuadFilter(EFilterType type) {
  constexpr std::array<Vert, 4> verts{{
      {{0.f, 0.f, 0.f}},
      {{0.f, 1.f, 0.f}},
      {{1.f, 0.f, 0.f}},
      {{1.f, 1.f, 0.f}},
  }};

  m_vbo = hsh::create_vertex_buffer(verts);
  m_uniBuf = hsh::create_dynamic_uniform_buffer<Uniform>();

  m_dataBind.hsh_bind(CColoredQuadFilterPipeline<type>(m_vbo.get(), m_uniBuf.get()));
}

void CColoredQuadFilter::draw(const zeus::CColor& color, const zeus::CRectangle& rect) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CColoredQuadFilter::draw", zeus::skMagenta);

#if !HSH_PROFILE_MODE
  m_uniform.m_matrix = zeus::CMatrix4f{};
  m_uniform.m_matrix[0][0] = rect.size.x() * 2.f;
  m_uniform.m_matrix[1][1] = rect.size.y() * 2.f;
  m_uniform.m_matrix[3][0] = rect.position.x() * 2.f - 1.f;
  m_uniform.m_matrix[3][1] = rect.position.y() * 2.f - 1.f;
  m_uniform.m_color = color;
  m_uniBuf.load(m_uniform);
#endif

  m_dataBind.draw(0, 4);
}

void CWideScreenFilter::draw(const zeus::CColor& color, float t) {
  if (g_Viewport.aspect < 1.7777f) {
    float targetHeight = g_Viewport.x8_width / 1.7777f;
    float delta = (g_Viewport.xc_height - targetHeight) * t / 2.f;
    delta /= float(g_Viewport.xc_height);
    zeus::CRectangle rect(0.f, 0.f, 1.f, delta);
    m_bottom.draw(color, rect);
    rect.position.y() = 1.f - delta;
    m_top.draw(color, rect);
  }
}

float CWideScreenFilter::SetViewportToMatch(float t) {
  if (g_Viewport.aspect < 1.7777f) {
    float targetHeight = g_Viewport.x8_width / 1.7777f;
    float delta = (g_Viewport.xc_height - targetHeight) * t / 2.f;
    hsh::viewport rect{};
    rect.width = g_Viewport.x8_width;
    rect.height = g_Viewport.xc_height - delta * 2.f;
    rect.y = delta;
    CGraphics::g_CroppedViewport = rect;
    CGraphics::g_SpareTexture.attach(rect);
    return 1.7777f;
  } else {
    SetViewportToFull();
    return g_Viewport.aspect;
  }
}

void CWideScreenFilter::SetViewportToFull() {
  hsh::viewport rect{};
  rect.width = g_Viewport.x8_width;
  rect.height = g_Viewport.xc_height;
  CGraphics::g_CroppedViewport = rect;
  CGraphics::g_SpareTexture.attach(rect);
}

} // namespace urde
