#include "Runtime/Graphics/Shaders/CAABoxShader.hpp"

#include <array>
#include <zeus/CAABox.hpp>

#include "Runtime/Graphics/CGraphics.hpp"
#include "CAABoxShader.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

template <bool zOnly>
struct CAABoxShaderPipeline
: pipeline<std::conditional_t<zOnly, NoColorAttachment<>, BlendAttachment<>>, depth_compare<hsh::LEqual>> {
  CAABoxShaderPipeline(hsh::vertex_buffer<CAABoxShader::Vert> vbo, hsh::uniform_buffer<CAABoxShader::Uniform> uniBuf) {
    this->position = uniBuf->m_xf * hsh::float4(vbo->m_pos, 1.f);
    this->color_out[0] = uniBuf->m_color;
  }
};
template struct CAABoxShaderPipeline<true>;
template struct CAABoxShaderPipeline<false>;

CAABoxShader::CAABoxShader(const zeus::CAABox& aabb, bool zOnly) {
  const std::array<Vert, 34> verts{{
      {{aabb.max.x(), aabb.max.y(), aabb.min.z()}}, {{aabb.max.x(), aabb.min.y(), aabb.min.z()}},
      {{aabb.max.x(), aabb.max.y(), aabb.max.z()}}, {{aabb.max.x(), aabb.min.y(), aabb.max.z()}},
      {{aabb.max.x(), aabb.min.y(), aabb.max.z()}},

      {{aabb.min.x(), aabb.max.y(), aabb.min.z()}}, {{aabb.min.x(), aabb.max.y(), aabb.min.z()}},
      {{aabb.max.x(), aabb.max.y(), aabb.min.z()}}, {{aabb.min.x(), aabb.max.y(), aabb.max.z()}},
      {{aabb.max.x(), aabb.max.y(), aabb.max.z()}}, {{aabb.max.x(), aabb.max.y(), aabb.max.z()}},

      {{aabb.min.x(), aabb.max.y(), aabb.min.z()}}, {{aabb.min.x(), aabb.max.y(), aabb.min.z()}},
      {{aabb.min.x(), aabb.min.y(), aabb.min.z()}}, {{aabb.min.x(), aabb.max.y(), aabb.max.z()}},
      {{aabb.min.x(), aabb.min.y(), aabb.max.z()}}, {{aabb.min.x(), aabb.min.y(), aabb.max.z()}},

      {{aabb.min.x(), aabb.min.y(), aabb.min.z()}}, {{aabb.min.x(), aabb.min.y(), aabb.min.z()}},
      {{aabb.max.x(), aabb.min.y(), aabb.min.z()}}, {{aabb.min.x(), aabb.min.y(), aabb.max.z()}},
      {{aabb.max.x(), aabb.min.y(), aabb.max.z()}}, {{aabb.max.x(), aabb.min.y(), aabb.max.z()}},

      {{aabb.min.x(), aabb.min.y(), aabb.max.z()}}, {{aabb.min.x(), aabb.min.y(), aabb.max.z()}},
      {{aabb.max.x(), aabb.min.y(), aabb.max.z()}}, {{aabb.min.x(), aabb.max.y(), aabb.max.z()}},
      {{aabb.max.x(), aabb.max.y(), aabb.max.z()}}, {{aabb.max.x(), aabb.max.y(), aabb.max.z()}},

      {{aabb.min.x(), aabb.min.y(), aabb.min.z()}}, {{aabb.min.x(), aabb.min.y(), aabb.min.z()}},
      {{aabb.max.x(), aabb.min.y(), aabb.min.z()}}, {{aabb.min.x(), aabb.max.y(), aabb.min.z()}},
      {{aabb.max.x(), aabb.max.y(), aabb.min.z()}},
  }};

  m_vbo = hsh::create_vertex_buffer(verts);
  m_uniBuf = hsh::create_dynamic_uniform_buffer<Uniform>();

  m_dataBind.hsh_bind(CAABoxShaderPipeline<zOnly>(m_vbo.get(), m_uniBuf.get()));
}

void CAABoxShader::draw(const zeus::CColor& color) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CAABoxShader::draw", zeus::skMagenta);

  m_uniform.m_xf = CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f();
  m_uniform.m_color = color;
  m_uniBuf.load(m_uniform);

  m_dataBind.draw(0, 34);
}

} // namespace urde
