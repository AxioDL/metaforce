#include "CNESShader.hpp"
#include "Graphics/CGraphics.hpp"

#include "CNESShader.cpp.hshhead"

namespace urde::MP1 {

using namespace hsh::pipeline;

struct NESPipeline : public pipeline<color_attachment<>> {
  NESPipeline(hsh::vertex_buffer<TexUVVert> vbo, hsh::uniform_buffer<ViewBlock> uniBuf, hsh::texture2d tex) {
    position = uniBuf->m_mv * hsh::float4(vbo->m_pos, 1.0);
    color_out[0] = uniBuf->m_color * tex.sample<float>(vbo->m_uv);
  }
};

void CNESShader::BuildShaderDataBinding(hsh::binding& binding, hsh::vertex_buffer<TexUVVert> vbo,
                                        hsh::uniform_buffer<ViewBlock> uniBuf, hsh::texture2d tex) {
  binding.hsh_bind(NESPipeline(vbo, uniBuf, tex));
}

} // namespace urde::MP1
