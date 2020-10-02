#include "Runtime/Graphics/Shaders/CThermalColdFilter.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

#include "CThermalColdFilter.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

struct CThermalColdFilterPipeline : pipeline<topology<hsh::TriangleStrip>,
                                             ERglBlendModeAttachment<ERglBlendMode::Blend, ERglBlendFactor::One,
                                                                     ERglBlendFactor::Zero, ERglLogicOp::Clear, true>,
                                             depth_write<false>> {
  CThermalColdFilterPipeline(hsh::vertex_buffer<CThermalColdFilter::Vert> vbo,
                             hsh::uniform_buffer<CThermalColdFilter::Uniform> ubo, hsh::render_texture2d sceneTex,
                             hsh::texture2d noiseTex) {
    static hsh::float4 kRGBToYPrime = {0.257f, 0.504f, 0.098f, 0.f};

    this->position = hsh::float4(vbo->m_pos, 0.f, 1.f);
    hsh::float4 noiseTexel = noiseTex.read<float>(Lookup8BPP(vbo->m_uvNoise, ubo->m_randOff));
    hsh::float2 indCoord = (hsh::float3x3(ubo->m_indMtx[0].xyz(), ubo->m_indMtx[1].xyz(), ubo->m_indMtx[2].xyz()) *
                            hsh::float3(noiseTexel.x - 0.5f, noiseTexel.w - 0.5f, 1.f))
                               .xy();
    hsh::float2 sceneUv = vbo->m_uv + indCoord;
    sceneUv.y = 1.f - sceneUv.y;
    float indScene = hsh::dot(sceneTex.sample<float>(sceneUv), kRGBToYPrime) + 16.f / 255.f;
    this->color_out[0] = ubo->m_colorRegs[0] * indScene + ubo->m_colorRegs[2] - ubo->m_colorRegs[1] * noiseTexel.x;
    this->color_out[0].w = ubo->m_colorRegs[1].x + ubo->m_colorRegs[1].w * noiseTexel.x + ubo->m_colorRegs[2].w;
  }

  static constexpr hsh::uint2 Lookup8BPP(hsh::float2 uv, float randOff) {
    int bx = int(uv.x) >> 3;
    int rx = int(uv.x) & 0x7;
    int by = int(uv.y) >> 2;
    int ry = int(uv.y) & 0x3;
    int bidx = by * 128 + bx;
    int addr = bidx * 32 + ry * 8 + rx + int(randOff);
    return hsh::uint2(addr & 0x3ff, addr >> 10);
  }
};

CThermalColdFilter::CThermalColdFilter() {
  const std::array<Vert, 4> verts{{
      {{-1.f, -1.f}, {0.f, 0.f}, {0.f, 0.f}},
      {{-1.f, 1.f}, {0.f, 1.f}, {0.f, 448.f}},
      {{1.f, -1.f}, {1.f, 0.f}, {640.f, 0.f}},
      {{1.f, 1.f}, {1.f, 1.f}, {640.f, 448.f}},
  }};
  m_vbo = hsh::create_vertex_buffer(verts);
  m_uniBuf = hsh::create_dynamic_uniform_buffer<Uniform>();
  m_dataBind.hsh_bind(CThermalColdFilterPipeline(m_vbo.get(), m_uniBuf.get(), CGraphics::g_SpareTexture.get_color(0),
                                                 g_Renderer->GetRandomStaticEntropyTex()));

  setNoiseOffset(0);
  setScale(0.f);
}

void CThermalColdFilter::draw() {
  SCOPED_GRAPHICS_DEBUG_GROUP("CThermalColdFilter::draw", zeus::skMagenta);

  CGraphics::ResolveSpareTexture(CGraphics::g_CroppedViewport);
  m_uniBuf.load(m_uniform);
  m_dataBind.draw(0, 4);
}

} // namespace urde
