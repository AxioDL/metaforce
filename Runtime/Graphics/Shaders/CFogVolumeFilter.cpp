#include "Runtime/Graphics/Shaders/CFogVolumeFilter.hpp"

#include <array>
#include <zeus/CColor.hpp>

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

#include "CFogVolumeFilter.cpp.hshhead"

namespace metaforce {
using namespace hsh::pipeline;

template <bool TwoWay>
struct CFogVolumeFilterPipeline
// FIXME replace BlendAttachment with ERglBlendFactor DstAlpha:One equivalent
: pipeline<topology<hsh::TriangleStrip>, std::conditional_t<TwoWay, SubtractAttachment<>, BlendAttachment<>>,
           depth_compare<hsh::Always>, depth_write<false>> {
  CFogVolumeFilterPipeline(hsh::vertex_buffer<CFogVolumeFilter::Vert> vbo,
                           hsh::uniform_buffer<CFogVolumeFilter::Uniform> uniBuf, hsh::render_texture2d frontfaceTex,
                           hsh::render_texture2d backfaceTex, hsh::texture2d linearizer) {
    this->position = hsh::float4(vbo->m_pos.x, -vbo->m_pos.y, 0.f, 1.f);
    const float linScale = 65535.f / 65536.f * 256.f;
    const float uvBias = 0.5f / 256.f;
    if constexpr (TwoWay) {
      float frontY;
      float backY;
      float frontX = hsh::modf((1.f - frontfaceTex.sample<float>(vbo->m_uv).x) * linScale, frontY);
      float backX = hsh::modf((1.f - backfaceTex.sample<float>(vbo->m_uv).x) * linScale, backY);
      float frontLin =
          linearizer.sample<float>(hsh::float2(frontX * 255.f / 256.f + uvBias, frontY / 256.f + uvBias)).x;
      float backLin = linearizer.sample<float>(hsh::float2(backX * 255.f / 256.f + uvBias, backY / 256.f + uvBias)).x;
      this->color_out[0] = hsh::float4(uniBuf->m_color.xyz(), (frontLin - backLin) * 10.f);
    } else {
      float y;
      float x = hsh::modf((1.f - frontfaceTex.sample<float>(vbo->m_uv).x) * linScale, y);
      float alpha = linearizer.sample<float>(hsh::float2(x * 255.f / 256.f + uvBias, y / 256.f + uvBias)).x * 10.f;
      this->color_out[0] = uniBuf->m_color * alpha;
    }
  }
};
template struct CFogVolumeFilterPipeline<true>;
template struct CFogVolumeFilterPipeline<false>;

CFogVolumeFilter::CFogVolumeFilter() {
  constexpr std::array<Vert, 4> verts{{
      {{-1.0, -1.0}, {0.0, 0.0}},
      {{-1.0, 1.0}, {0.0, 1.0}},
      {{1.0, -1.0}, {1.0, 0.0}},
      {{1.0, 1.0}, {1.0, 1.0}},
  }};
  m_vbo = hsh::create_vertex_buffer(verts);
  m_uniBuf = hsh::create_dynamic_uniform_buffer<Uniform>();

  // FIXME hsh bug: can't bind all constant values
  bool twoWay = false;
  m_dataBind1Way.hsh_1way_bind(
      CFogVolumeFilterPipeline<twoWay>(m_vbo.get(), m_uniBuf.get(), CGraphics::g_SpareTexture.get_depth(0),
                                       CGraphics::g_SpareTexture.get_depth(1), g_Renderer->GetFogRampTex()));
  twoWay = true;
  m_dataBind2Way.hsh_2way_bind(
      CFogVolumeFilterPipeline<twoWay>(m_vbo.get(), m_uniBuf.get(), CGraphics::g_SpareTexture.get_depth(0),
                                       CGraphics::g_SpareTexture.get_depth(1), g_Renderer->GetFogRampTex()));
}

void CFogVolumeFilter::draw2WayPass(const zeus::CColor& color) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CFogVolumeFilter::draw2WayPass", zeus::skMagenta);

#if !HSH_PROFILE_MODE
  m_uniBuf.load({color});
#endif
  m_dataBind2Way.draw(0, 4);
}

void CFogVolumeFilter::draw1WayPass(const zeus::CColor& color) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CFogVolumeFilter::draw1WayPass", zeus::skMagenta);

#if !HSH_PROFILE_MODE
  m_uniBuf.load({color});
#endif
  m_dataBind1Way.draw(0, 4);
}

} // namespace metaforce
