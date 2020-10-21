#include "Runtime/Graphics/Shaders/CMapSurfaceShader.hpp"

#include "Runtime/Graphics/CGraphics.hpp"

#include "CMapSurfaceShader.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

struct CMapSurfaceShaderPipeline : pipeline<topology<hsh::TriangleStrip>, BlendAttachment<>, depth_compare<hsh::GEqual>,
                                            depth_write<false>, cull_mode<hsh::CullBack>> {
  CMapSurfaceShaderPipeline(hsh::uniform_buffer<CMapSurfaceShader::Uniform> uniBuf,
                            hsh::vertex_buffer<CMapSurfaceShader::Vert> vbo, hsh::index_buffer<uint32_t> ibo) {
    this->position = uniBuf->mtx * hsh::float4(vbo->pos, 1.f);
    this->color_out[0] = uniBuf->color;
  }
};

CMapSurfaceShader::CMapSurfaceShader(hsh::vertex_buffer<Vert> vbo, hsh::index_buffer<uint32_t> ibo) {
  m_uniBuf = hsh::create_dynamic_uniform_buffer<Uniform>();
  m_dataBind.hsh_bind(CMapSurfaceShaderPipeline(m_uniBuf.get(), vbo, ibo));
}

void CMapSurfaceShader::draw(const zeus::CColor& color, u32 start, u32 count) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CMapSurfaceShader::draw", zeus::skMagenta);
#if !HSH_PROFILE_MODE
  m_uniBuf.load({CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f(), color});
#endif
  m_dataBind.draw_indexed(start, count);
}

} // namespace urde
