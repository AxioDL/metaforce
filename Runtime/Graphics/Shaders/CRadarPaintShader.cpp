#include "Runtime/Graphics/Shaders/CRadarPaintShader.hpp"

#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Graphics/CTexture.hpp"

#include "CRadarPaintShader.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

struct CRadarPaintShaderPipeline : pipeline<topology<hsh::TriangleStrip>, AdditiveAttachment<>, depth_write<false>> {
  CRadarPaintShaderPipeline(hsh::vertex_buffer<CRadarPaintShader::Instance> vbo HSH_VAR_INSTANCE,
                            hsh::uniform_buffer<CRadarPaintShader::Uniform> ubo, hsh::texture2d tex) {
    this->position = ubo->xf * hsh::float4(vbo->pos[this->vertex_id], 1.f);
    this->color_out[0] = vbo->color * tex.sample<float>(vbo->uv[this->vertex_id]);
  }
};

void CRadarPaintShader::draw(const std::vector<Instance>& instances, const CTexture* tex) {
  if (instances.empty()) {
    return;
  }

  SCOPED_GRAPHICS_DEBUG_GROUP("CRadarPaintShader::draw", zeus::skMagenta);

  if (instances.size() > m_maxInsts) {
    m_maxInsts = instances.size();
    m_tex = tex;
    m_vbo = hsh::create_dynamic_vertex_buffer<Instance>(m_maxInsts);
    m_uniBuf = hsh::create_dynamic_uniform_buffer<Uniform>();
    hsh::texture2d tex2d = m_tex->GetBooTexture();
    m_dataBind.hsh_bind(CRadarPaintShaderPipeline(m_vbo.get(), m_uniBuf.get(), tex2d));
  }

#if !HSH_PROFILE_MODE
  m_uniBuf.load({CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f()});
  m_vbo.load(instances);
#endif
  m_dataBind.draw_instanced(0, 4, instances.size());
}

} // namespace urde
