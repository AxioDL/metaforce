#include "Runtime/Graphics/Shaders/CXRayBlurFilter.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Graphics/CTexture.hpp"

#include "CXRayBlurFilter.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

struct CXRayBlurFilterPipeline
: pipeline<topology<hsh::TriangleStrip>,
           color_attachment<hsh::One, hsh::Zero, hsh::Add, hsh::One, hsh::Zero, hsh::Add,
                            hsh::ColorComponentFlags(hsh::CC_Red | hsh::CC_Green | hsh::CC_Blue)>,
           depth_write<false>> {
  CXRayBlurFilterPipeline(hsh::vertex_buffer<CXRayBlurFilter::Vert> vbo,
                          hsh::uniform_buffer<CXRayBlurFilter::Uniform> ubo, hsh::render_texture2d sceneTex,
                          hsh::texture2d paletteTex) {
    hsh::float2 uv0 = (ubo->m_uv[0] * hsh::float4(vbo->m_uv, 0.f, 1.f)).xy();
    hsh::float2 uv1 = (ubo->m_uv[1] * hsh::float4(vbo->m_uv, 0.f, 1.f)).xy();
    hsh::float2 uv2 = (ubo->m_uv[2] * hsh::float4(vbo->m_uv, 0.f, 1.f)).xy();
    hsh::float2 uv3 = (ubo->m_uv[3] * hsh::float4(vbo->m_uv, 0.f, 1.f)).xy();
    hsh::float2 uv4 = (ubo->m_uv[4] * hsh::float4(vbo->m_uv, 0.f, 1.f)).xy();
    hsh::float2 uv5 = (ubo->m_uv[5] * hsh::float4(vbo->m_uv, 0.f, 1.f)).xy();
    hsh::float2 uv6 = (ubo->m_uv[6] * hsh::float4(vbo->m_uv, 0.f, 1.f)).xy();
    hsh::float2 uv7 = (ubo->m_uv[7] * hsh::float4(vbo->m_uv, 0.f, 1.f)).xy();
    position = hsh::float4(vbo->m_pos, 0.f, 1.f);

    hsh::float4 kRGBToYPrime{0.257f, 0.504f, 0.098f, 0.f};
    hsh::float2 uvSample = hsh::float2(hsh::dot(sceneTex.sample<float>(uv0), kRGBToYPrime) * 0.98f + 0.01f, 0.5f);
    hsh::float4 colorSample = paletteTex.sample<float>(uvSample) * 0.125f;
    uvSample = hsh::float2(hsh::dot(sceneTex.sample<float>(uv1), kRGBToYPrime) * 0.98f + 0.01f, 0.5f);
    colorSample += paletteTex.sample<float>(uvSample) * 0.125f;
    uvSample = hsh::float2(hsh::dot(sceneTex.sample<float>(uv2), kRGBToYPrime) * 0.98f + 0.01f, 0.5f);
    colorSample += paletteTex.sample<float>(uvSample) * 0.125f;
    uvSample = hsh::float2(hsh::dot(sceneTex.sample<float>(uv3), kRGBToYPrime) * 0.98f + 0.01f, 0.5f);
    colorSample += paletteTex.sample<float>(uvSample) * 0.125f;
    uvSample = hsh::float2(hsh::dot(sceneTex.sample<float>(uv4), kRGBToYPrime) * 0.98f + 0.01f, 0.5f);
    colorSample += paletteTex.sample<float>(uvSample) * 0.125f;
    uvSample = hsh::float2(hsh::dot(sceneTex.sample<float>(uv5), kRGBToYPrime) * 0.98f + 0.01f, 0.5f);
    colorSample += paletteTex.sample<float>(uvSample) * 0.125f;
    uvSample = hsh::float2(hsh::dot(sceneTex.sample<float>(uv6), kRGBToYPrime) * 0.98f + 0.01f, 0.5f);
    colorSample += paletteTex.sample<float>(uvSample) * 0.125f;
    uvSample = hsh::float2(hsh::dot(sceneTex.sample<float>(uv7), kRGBToYPrime) * 0.98f + 0.01f, 0.5f);
    colorSample += paletteTex.sample<float>(uvSample) * 0.125f;
    color_out[0] = colorSample;
  }
};

CXRayBlurFilter::CXRayBlurFilter(TLockedToken<CTexture>& tex) : m_paletteTex(tex) {
  constexpr std::array<Vert, 4> verts{{
      {{-1.f, -1.f}, {0.f, 0.f}},
      {{-1.f, 1.f}, {0.f, 1.f}},
      {{1.f, -1.f}, {1.f, 0.f}},
      {{1.f, 1.f}, {1.f, 1.f}},
  }};
  m_vbo = hsh::create_vertex_buffer<Vert, 4>(verts);
  m_uniBuf = hsh::create_dynamic_uniform_buffer<Uniform>();
  hsh::texture2d paletteTex = m_paletteTex->GetPaletteTexture();
  m_dataBind.hsh_bind(
      CXRayBlurFilterPipeline(m_vbo.get(), m_uniBuf.get(), CGraphics::g_SpareTexture.get_color(0), paletteTex));
}

void CXRayBlurFilter::draw(float amount) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CXRayBlurFilter::draw", zeus::skMagenta);

  CGraphics::ResolveSpareTexture(CGraphics::g_CroppedViewport);

  const float blurL = amount * g_tweakGui->GetXrayBlurScaleLinear() * 0.25f;
  const float blurQ = amount * g_tweakGui->GetXrayBlurScaleQuadratic() * 0.25f;

  for (size_t i = 0; i < m_uniform.m_uv.size(); ++i) {
    const float iflt = float(i) / 2.f;
    const float uvScale = (1.f - (blurL * iflt + blurQ * iflt * iflt));
    const float uvOffset = uvScale * -0.5f + 0.5f;
    m_uniform.m_uv[i][0][0] = uvScale;
    m_uniform.m_uv[i][1][1] = uvScale;
    m_uniform.m_uv[i][3][0] = uvOffset;
    m_uniform.m_uv[i][3][1] = uvOffset;
  }

  m_uniBuf.load(m_uniform);
  m_dataBind.draw(0, 4);
}

} // namespace urde
