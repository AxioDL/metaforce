#include "Runtime/Graphics/Shaders/CCameraBlurFilter.hpp"

#include <algorithm>
#include <cmath>

#include "Runtime/Graphics/CGraphics.hpp"

#include "CCameraBlurFilter.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

struct CCameraBlurFilterPipeline : pipeline<color_attachment<>> {
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
  if (amount <= 0.f)
    return;
  SCOPED_GRAPHICS_DEBUG_GROUP("CCameraBlurFilter::draw", zeus::skMagenta);

  SClipScreenRect clipRect(g_Viewport);
  CGraphics::ResolveSpareTexture(clipRect, 0, clearDepth);
  float aspect = CGraphics::g_CroppedViewport.xc_width / float(CGraphics::g_CroppedViewport.x10_height);

  float xFac = CGraphics::g_CroppedViewport.xc_width / float(g_Viewport.x8_width);
  float yFac = CGraphics::g_CroppedViewport.x10_height / float(g_Viewport.xc_height);
  float xBias = CGraphics::g_CroppedViewport.x4_left / float(g_Viewport.x8_width);
  float yBias = CGraphics::g_CroppedViewport.x8_top / float(g_Viewport.xc_height);

  Vert verts[4] = {
      {{-1.0, -1.0}, {xBias, yBias}},
      {{-1.0, 1.0}, {xBias, yBias + yFac}},
      {{1.0, -1.0}, {xBias + xFac, yBias}},
      {{1.0, 1.0}, {xBias + xFac, yBias + yFac}},
  };
  m_vbo.load(verts);

  for (int i = 0; i < 6; ++i) {
    float tmp = i;
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
  m_uniBuf.load(m_uniform);
  m_dataBind.draw(0, 4);
}

} // namespace urde
