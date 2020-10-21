#include "Runtime/Graphics/Shaders/CWorldShadowShader.hpp"

#include <array>

#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

#include "CWorldShadowShader.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

template <bool Depth>
struct CWorldShadowShaderPipeline : pipeline<topology<hsh::TriangleStrip>, BlendAttachment<>, depth_write<Depth>,
                                            depth_compare<Depth ? hsh::LEqual : hsh::Always>> {
  CWorldShadowShaderPipeline(hsh::vertex_buffer<CWorldShadowShader::Vert> vbo,
                            hsh::uniform_buffer<CWorldShadowShader::Uniform> ubo) {
    this->position = ubo->m_matrix * hsh::float4(vbo->m_pos, 1.f);
    this->color_out[0] = ubo->m_color;
  }
};
template struct CWorldShadowShaderPipeline<true>;
template struct CWorldShadowShaderPipeline<false>;

CWorldShadowShader::CWorldShadowShader(u32 w, u32 h) : m_w(w), m_h(h) {
  m_vbo = hsh::create_dynamic_vertex_buffer<Vert>(4);
  m_uniBuf = hsh::create_dynamic_uniform_buffer<Uniform>();
  m_tex = hsh::create_render_texture2d({m_w, m_h}, 1, 0);

  // FIXME hsh bug: can't bind all constant values
  bool depth = false;
  m_dataBind.hsh_bind(CWorldShadowShaderPipeline<depth>(m_vbo.get(), m_uniBuf.get()));
  depth = true;
  m_zDataBind.hsh_z_bind(CWorldShadowShaderPipeline<depth>(m_vbo.get(), m_uniBuf.get()));
}

void CWorldShadowShader::bindRenderTarget() { m_tex.attach(); }

void CWorldShadowShader::drawBase(float extent) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CWorldShadowShader::drawBase", zeus::skMagenta);

#if !HSH_PROFILE_MODE
  const std::array<Vert, 4> verts{{
      {{-extent, 0.f, extent}},
      {{extent, 0.f, extent}},
      {{-extent, 0.f, -extent}},
      {{extent, 0.f, -extent}},
  }};
  m_vbo.load(verts);

  m_uniform.m_matrix = CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f();
  m_uniform.m_color = zeus::skWhite;
  m_uniBuf.load(m_uniform);
#endif

  m_zDataBind.draw(0, 4);
}

void CWorldShadowShader::lightenShadow() {
  SCOPED_GRAPHICS_DEBUG_GROUP("CWorldShadowShader::lightenShadow", zeus::skMagenta);

#if !HSH_PROFILE_MODE
  m_uniform.m_color = zeus::CColor(1.f, 0.25f);
  m_uniBuf.load(m_uniform);
#endif

  m_dataBind.draw(0, 4);
}

void CWorldShadowShader::blendPreviousShadow() {
  SCOPED_GRAPHICS_DEBUG_GROUP("CWorldShadowShader::blendPreviousShadow", zeus::skMagenta);

  if (!m_prevQuad) {
    m_prevQuad.emplace(EFilterType::Blend, m_tex.get_color(0));
  }
  zeus::CRectangle rect(0.f, 1.f, 1.f, -1.f);
  m_prevQuad->draw({1.f, 0.85f}, 1.f, rect);
}

void CWorldShadowShader::resolveTexture() { m_tex.resolve_color_binding(0, hsh::rect2d{{}, {m_w, m_h}}, false); }

} // namespace urde
