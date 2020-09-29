#include "Runtime/Graphics/Shaders/CColoredStripShader.hpp"

#include <array>

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

#include "CColoredStripShader.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

template <CColoredStripShader::Mode Mode>
struct CColoredStripShaderColorAttachment : color_attachment<> {};
template <>
struct CColoredStripShaderColorAttachment<CColoredStripShader::Mode::Alpha> : BlendAttachment<> {};
template <>
struct CColoredStripShaderColorAttachment<CColoredStripShader::Mode::Additive> : AdditiveAttachment<> {};
// TODO following are wrong
template <>
struct CColoredStripShaderColorAttachment<CColoredStripShader::Mode::FullAdditive> : MultiplyAttachment<> {};
template <>
struct CColoredStripShaderColorAttachment<CColoredStripShader::Mode::Subtractive> : SubtractAttachment<> {};

template <CColoredStripShader::Mode Mode>
// TODO typename == hsh bug?
struct CColoredStripShaderPipeline
: pipeline<topology<hsh::TriangleStrip>, typename CColoredStripShaderColorAttachment<Mode>::color_attachment,
           depth_compare<hsh::LEqual>, depth_write<false>> {
  CColoredStripShaderPipeline(hsh::vertex_buffer<CColoredStripShader::Vert> vbo,
                              hsh::uniform_buffer<CColoredStripShader::Uniform> uniBuf, hsh::texture2d tex) {
    this->position = uniBuf->m_matrix * hsh::float4(vbo->m_pos, 1.f);
    this->color_out[0] = vbo->m_color * uniBuf->m_color * tex.sample<float>(vbo->m_uv);
  }
};
template struct CColoredStripShaderPipeline<CColoredStripShader::Mode::Alpha>;
template struct CColoredStripShaderPipeline<CColoredStripShader::Mode::Additive>;
template struct CColoredStripShaderPipeline<CColoredStripShader::Mode::FullAdditive>;
template struct CColoredStripShaderPipeline<CColoredStripShader::Mode::Subtractive>;

CColoredStripShader::CColoredStripShader(size_t maxVerts, Mode mode, hsh::texture2d tex) {
  m_vbo = hsh::create_dynamic_vertex_buffer<Vert>(maxVerts);
  m_uniBuf = hsh::create_dynamic_uniform_buffer<Uniform>();

  if (!tex) {
    tex = g_Renderer->GetWhiteTexture();
  }
  m_dataBind.hsh_bind(CColoredStripShaderPipeline<mode>(m_vbo.get(), m_uniBuf.get(), tex));
}

void CColoredStripShader::draw(const zeus::CColor& color, size_t numVerts, const Vert* verts) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CColoredStripShader::draw", zeus::skMagenta);

  m_vbo.load(hsh::detail::ArrayProxy{verts, numVerts});

  m_uniform.m_matrix = CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f();
  m_uniform.m_color = color;
  m_uniBuf.load(m_uniform);

  m_dataBind.draw(0, numVerts);
}

} // namespace urde
