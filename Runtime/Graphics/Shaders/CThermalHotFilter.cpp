#include "Runtime/Graphics/Shaders/CThermalHotFilter.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

#include "CThermalHotFilter.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

struct CThermalHotFilterPipeline
: pipeline<ERglBlendModeAttachment<ERglBlendMode::Blend, ERglBlendFactor::DstAlpha, ERglBlendFactor::InvDstAlpha,
                                   ERglLogicOp::Clear, true>,
           depth_write<false>> {
  CThermalHotFilterPipeline(hsh::vertex_buffer<CThermalHotFilter::Vert> vbo,
                            hsh::uniform_buffer<CThermalHotFilter::Uniform> ubo, hsh::render_texture2d sceneTex,
                            hsh::texture2d paletteTex) {
    static hsh::float4 kRGBToYPrime = {0.257f, 0.504f, 0.098f, 0.f};

    this->position = hsh::float4(vbo->m_pos, 0.f, 1.f);

    hsh::float2 sceneUv = vbo->m_uv;
    sceneUv.y = 1.f - sceneUv.y;
    float sceneSample = hsh::dot(sceneTex.sample<float>(sceneUv), kRGBToYPrime) + 16.f / 255.f;
    hsh::float4 colorSample = paletteTex.sample<float>(hsh::float2(sceneSample / 16.f, 0.5f));
    this->color_out[0] = hsh::float4(colorSample.xyz(), 0.f);
  }
};

CThermalHotFilter::CThermalHotFilter() {
  const std::array<Vert, 4> verts{{
      {{-1.0, -1.0}, {0.0, 0.0}},
      {{-1.0, 1.0}, {0.0, 1.0}},
      {{1.0, -1.0}, {1.0, 0.0}},
      {{1.0, 1.0}, {1.0, 1.0}},
  }};
  m_vbo = hsh::create_vertex_buffer(verts);
  m_uniBuf = hsh::create_dynamic_uniform_buffer<Uniform>();
  m_dataBind.hsh_bind(CThermalHotFilterPipeline(m_vbo.get(), m_uniBuf.get(), CGraphics::g_SpareTexture.get_color(0),
                                                g_Renderer->GetThermoPalette()));
}

void CThermalHotFilter::draw() {
  SCOPED_GRAPHICS_DEBUG_GROUP("CThermalHotFilter::draw", zeus::skMagenta);

#if !HSH_PROFILE_MODE
  CGraphics::ResolveSpareTexture(CGraphics::g_CroppedViewport);
  m_uniBuf.load(m_uniform);
#endif
  m_dataBind.draw(0, 4);
}

} // namespace urde
