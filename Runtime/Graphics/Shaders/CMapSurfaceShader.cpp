#include "Runtime/Graphics/Shaders/CMapSurfaceShader.hpp"

#include <array>

#include "Runtime/Graphics/CGraphics.hpp"

namespace urde {

CMapSurfaceShader::CMapSurfaceShader(hsh::vertex_buffer<Vert> vbo,
                                     hsh::index_buffer<uint32_t> ibo) {
  m_uniBuf = hsh::create_dynamic_uniform_buffer<Uniform>();
  m_dataBind.hsh_bind(m_uniBuf.get(), vbo, ibo);
}

void CMapSurfaceShader::draw(const zeus::CColor& color, u32 start, u32 count) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CMapSurfaceShader::draw", zeus::skMagenta);
  Uniform uniform = {CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f(), color};
  m_uniBuf.load(uniform);
  m_dataBind.draw_indexed(start, count);
}

} // namespace urde
