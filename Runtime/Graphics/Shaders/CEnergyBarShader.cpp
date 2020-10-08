#include "Runtime/Graphics/Shaders/CEnergyBarShader.hpp"

#include "Runtime/Graphics/CGraphics.hpp"

#include "CEnergyBarShader.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

struct CEnergyBarShaderPipeline
: pipeline<topology<hsh::TriangleStrip>, AdditiveAttachment<>, depth_compare<hsh::LEqual>, depth_write<false>> {
  CEnergyBarShaderPipeline(hsh::vertex_buffer<CEnergyBarShader::Vertex> vbo,
                           hsh::uniform_buffer<CEnergyBarShader::Uniform> uniBuf, hsh::texture2d tex) {
    this->position = uniBuf->m_matrix * hsh::float4(vbo->pos, 1.f);
    this->color_out[0] = uniBuf->m_color * tex.sample<float>(vbo->uv);
  }
};

void CEnergyBarShader::updateModelMatrix() {
  m_uniform.m_matrix = CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f();
}

void CEnergyBarShader::draw(const zeus::CColor& color0, const std::vector<Vertex>& verts0, const zeus::CColor& color1,
                            const std::vector<Vertex>& verts1, const zeus::CColor& color2,
                            const std::vector<Vertex>& verts2, hsh::texture2d tex) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CEnergyBarShader::draw", zeus::skMagenta);

  size_t totalVerts = verts0.size() + verts1.size() + verts2.size();
  if (totalVerts == 0) {
    return;
  }

  if (totalVerts > m_maxVerts) {
    m_maxVerts = totalVerts;

    m_vbo = hsh::create_dynamic_vertex_buffer<Vertex>(m_maxVerts);
    for (size_t i = 0; i < m_uniBuf.size(); ++i) {
      if (!m_uniBuf[i]) {
        m_uniBuf[i] = hsh::create_dynamic_uniform_buffer<Uniform>();
      }
      m_dataBind[i].hsh_bind(CEnergyBarShaderPipeline(m_vbo.get(), m_uniBuf[i].get(), tex));
    }
  }

  size_t vertIter = 0;
  Vertex* verts = m_vbo.map();
  if (!verts0.empty()) {
    std::memmove(verts, verts0.data(), sizeof(Vertex) * verts0.size());
    vertIter += verts0.size();
  }
  if (!verts1.empty()) {
    std::memmove(verts + vertIter, verts1.data(), sizeof(Vertex) * verts1.size());
    vertIter += verts1.size();
  }
  if (!verts2.empty()) {
    std::memmove(verts + vertIter, verts2.data(), sizeof(Vertex) * verts2.size());
  }
  m_vbo.unmap();

  vertIter = 0;
  if (!verts0.empty()) {
    m_uniform.m_color = color0;
    m_uniBuf[0].load(m_uniform);
    m_dataBind[0].draw(0, verts0.size());
    vertIter += verts0.size();
  }
  if (!verts1.empty()) {
    m_uniform.m_color = color1;
    m_uniBuf[1].load(m_uniform);
    m_dataBind[1].draw(vertIter, verts1.size());
    vertIter += verts1.size();
  }
  if (!verts2.empty()) {
    m_uniform.m_color = color2;
    m_uniBuf[2].load(m_uniform);
    m_dataBind[2].draw(vertIter, verts2.size());
  }
}

} // namespace urde
