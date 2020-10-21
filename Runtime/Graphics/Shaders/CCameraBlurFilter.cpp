#include "Runtime/Graphics/Shaders/CCameraBlurFilter.hpp"

#include <array>
#include <cmath>

#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

#include "CCameraBlurFilter.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

struct CCameraBlurFilterPipeline : FilterPipeline<EFilterType::Blend> {
  CCameraBlurFilterPipeline(hsh::vertex_buffer<CCameraBlurFilter::Vert> vbo,
                            hsh::uniform_buffer<CCameraBlurFilter::Uniform> ubo,
                            hsh::render_texture2d tex) {
    position = hsh::float4(vbo->m_pos, 0.f, 1.f);

    hsh::float4 colorSample = tex.sample<float>(vbo->m_uv) * 0.14285715f;
    colorSample += tex.sample<float>(vbo->m_uv + ubo->m_uv[0]) * 0.14285715f;
    colorSample += tex.sample<float>(vbo->m_uv + ubo->m_uv[1]) * 0.14285715f;
    colorSample += tex.sample<float>(vbo->m_uv + ubo->m_uv[2]) * 0.14285715f;
    colorSample += tex.sample<float>(vbo->m_uv + ubo->m_uv[3]) * 0.14285715f;
    colorSample += tex.sample<float>(vbo->m_uv + ubo->m_uv[4]) * 0.14285715f;
    colorSample += tex.sample<float>(vbo->m_uv + ubo->m_uv[5]) * 0.14285715f;
    color_out[0] = hsh::float4(colorSample.xyz(), ubo->m_opacity);
  }
};

CCameraBlurFilter::CCameraBlurFilter() {
  m_vbo = hsh::create_dynamic_vertex_buffer<Vert>(4);
  m_uniBuf = hsh::create_dynamic_uniform_buffer<Uniform>();
  m_dataBind.hsh_bind(CCameraBlurFilterPipeline(m_vbo.get(), m_uniBuf.get(), CGraphics::g_SpareTexture.get_color(0)));
}

void CCameraBlurFilter::draw(float amount, bool clearDepth) {
  if (amount <= 0.f) {
    return;
  }

  SCOPED_GRAPHICS_DEBUG_GROUP("CCameraBlurFilter::draw", zeus::skMagenta);

  const SClipScreenRect clipRect(g_Viewport);
  CGraphics::ResolveSpareTexture(clipRect, 0, clearDepth);
  const float aspect = float(CGraphics::g_CroppedViewport.xc_width) / float(CGraphics::g_CroppedViewport.x10_height);

  const float xFac = float(CGraphics::g_CroppedViewport.xc_width) / float(g_Viewport.x8_width);
  const float yFac = float(CGraphics::g_CroppedViewport.x10_height) / float(g_Viewport.xc_height);
  const float xBias = float(CGraphics::g_CroppedViewport.x4_left) / float(g_Viewport.x8_width);
  const float yBias = float(CGraphics::g_CroppedViewport.x8_top) / float(g_Viewport.xc_height);

  const std::array<Vert, 4> verts{{
      {hsh::float2{-1.0, -1.0}, {xBias, yBias + yFac}},
      {hsh::float2{-1.0, 1.0}, {xBias, yBias}},
      {hsh::float2{1.0, -1.0}, {xBias + xFac, yBias + yFac}},
      {hsh::float2{1.0, 1.0}, {xBias + xFac, yBias}},
  }};
#if !HSH_PROFILE_MODE
  m_vbo.load(verts);
#endif

  // m_uniform.m_uv.size()
  for (size_t i = 0; i < 6; ++i) {
    auto tmp = static_cast<float>(i);
    tmp *= 2.f * M_PIF;
    tmp /= 6.f;

    float amtX = std::cos(tmp);
    amtX *= amount / 448.f / aspect;

    float amtY = std::sin(tmp);
    amtY *= amount / 448.f;

    m_uniform.m_uv[i].x = amtX * xFac;
    m_uniform.m_uv[i].y = amtY * yFac;
  }
  m_uniform.m_opacity = std::min(amount / 2.f, 1.f);
#if !HSH_PROFILE_MODE
  m_uniBuf.load(m_uniform);
#endif
  m_dataBind.draw(0, 4);
}

} // namespace urde
