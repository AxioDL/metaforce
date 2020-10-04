#include "Runtime/Graphics/Shaders/CSpaceWarpFilter.hpp"

#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

#include "CSpaceWarpFilter.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

struct CSpaceWarpFilterPipeline
: pipeline<topology<hsh::TriangleStrip>,
           color_attachment<hsh::One, hsh::Zero, hsh::Add, hsh::One, hsh::Zero, hsh::Add,
                            hsh::ColorComponentFlags(hsh::CC_Red | hsh::CC_Green | hsh::CC_Blue)>,
           depth_write<false>> {
  CSpaceWarpFilterPipeline(hsh::vertex_buffer<CSpaceWarpFilter::Vert> vbo,
                           hsh::uniform_buffer<CSpaceWarpFilter::Uniform> ubo, hsh::render_texture2d sceneTex,
                           hsh::texture2d indTex) {
    this->position = ubo->m_matrix * hsh::float4(vbo->m_pos, 0.f, 1.f);
    hsh::float2 sceneUv = vbo->m_pos * hsh::float2(0.5f) + hsh::float2(0.5f);
    sceneUv.y = 1.f - sceneUv.y;
    hsh::float2 indUv = (hsh::float3x3(ubo->m_indXf[0].xyz(), ubo->m_indXf[1].xyz(), ubo->m_indXf[2].xyz()) *
                         hsh::float3(vbo->m_uv, 1.f))
                            .xy();
    indUv.y = 1.f - indUv.y;
    hsh::float2 indUvSample = indTex.sample<float>(indUv).xy() * hsh::float2(2.f) - hsh::float2(1.f - 1.f / 256.f);
    this->color_out[0] = hsh::float4(sceneTex.sample<float>(sceneUv + indUvSample + ubo->m_strength.xy()).xyz(), 1.f);
  }
};

void CSpaceWarpFilter::GenerateWarpRampTex() {
  constexpr int skWarpRampSize = 32;
  m_warpTex =
      hsh::create_texture2d({skWarpRampSize, skWarpRampSize}, hsh::RGBA8_UNORM, 1, [&](void* data, std::size_t size) {
        auto* buf = static_cast<CTexture::RGBA8*>(data);
        const float halfRes = skWarpRampSize / 2.f;
        for (int y = 0; y < skWarpRampSize + 1; ++y) {
          for (int x = 0; x < skWarpRampSize + 1; ++x) {
            zeus::CVector2f vec((x - halfRes) / halfRes, (y - halfRes) / halfRes);
            const float mag = vec.magnitude();
            if (mag < 1.f && vec.canBeNormalized()) {
              vec.normalize();
              vec *= zeus::CVector2f(std::sqrt(mag));
            }
            auto& pixel = buf[y * skWarpRampSize + x];
            pixel.a = zeus::clamp(0, int((((vec.x() / 2.f + 0.5f) - x / float(skWarpRampSize)) + 0.5f) * 255), 255);
            pixel.r = pixel.g = pixel.b =
                zeus::clamp(0, int((((vec.y() / 2.f + 0.5f) - y / float(skWarpRampSize)) + 0.5f) * 255), 255);
          }
        }
      });
}

CSpaceWarpFilter::CSpaceWarpFilter() {
  GenerateWarpRampTex();

  constexpr std::array<Vert, 4> verts{{
      {{-1.f, -1.f}, {0.f, 0.f}},
      {{-1.f, 1.f}, {0.f, 1.f}},
      {{1.f, -1.f}, {1.f, 0.f}},
      {{1.f, 1.f}, {1.f, 1.f}},
  }};
  m_vbo = hsh::create_vertex_buffer(verts);
  m_uniBuf = hsh::create_dynamic_uniform_buffer<Uniform>();

  m_dataBind.hsh_bind(
      CSpaceWarpFilterPipeline(m_vbo.get(), m_uniBuf.get(), CGraphics::g_SpareTexture.get_color(0), m_warpTex.get()));
}

void CSpaceWarpFilter::draw(const zeus::CVector3f& pt) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CSpaceWarpFilter::draw", zeus::skMagenta);

  /* Indirect coords are full-texture sampling when warp is completely in viewport */
  m_uniform.m_indXf[1][1] = 1.f;
  m_uniform.m_indXf[0][0] = 1.f;
  m_uniform.m_indXf[2][0] = 0.f;
  m_uniform.m_indXf[2][1] = 0.f;

  /* Warp effect is fixed at 192x192 rectangle in original (1/2.5 viewport height) */
  float aspect = CGraphics::g_CroppedViewport.xc_width / float(CGraphics::g_CroppedViewport.x10_height);
  m_uniform.m_matrix[1][1] = 1.f / 2.5f;
  m_uniform.m_matrix[0][0] = m_uniform.m_matrix[1][1] / aspect;

  SClipScreenRect clipRect = {};
  clipRect.x4_left = ((pt[0] - m_uniform.m_matrix[0][0]) / 2.f + 0.5f) * CGraphics::g_CroppedViewport.xc_width;
  if (clipRect.x4_left >= CGraphics::g_CroppedViewport.xc_width)
    return;
  clipRect.x8_top = ((pt[1] - m_uniform.m_matrix[1][1]) / 2.f + 0.5f) * CGraphics::g_CroppedViewport.x10_height;
  if (clipRect.x8_top >= CGraphics::g_CroppedViewport.x10_height)
    return;
  clipRect.xc_width = CGraphics::g_CroppedViewport.xc_width * m_uniform.m_matrix[0][0];
  if (clipRect.x4_left + clipRect.xc_width <= 0)
    return;
  clipRect.x10_height = CGraphics::g_CroppedViewport.x10_height * m_uniform.m_matrix[1][1];
  if (clipRect.x8_top + clipRect.x10_height <= 0)
    return;

  float oldW = clipRect.xc_width;
  if (clipRect.x4_left < 0) {
    clipRect.xc_width += clipRect.x4_left;
    m_uniform.m_indXf[0][0] = clipRect.xc_width / oldW;
    m_uniform.m_indXf[2][0] = -clipRect.x4_left / oldW;
    clipRect.x4_left = 0;
  }

  float oldH = clipRect.x10_height;
  if (clipRect.x8_top < 0) {
    clipRect.x10_height += clipRect.x8_top;
    m_uniform.m_indXf[1][1] = clipRect.x10_height / oldH;
    m_uniform.m_indXf[2][1] = -clipRect.x8_top / oldH;
    clipRect.x8_top = 0;
  }

  float tmp = clipRect.x4_left + clipRect.xc_width;
  if (tmp >= CGraphics::g_CroppedViewport.xc_width) {
    clipRect.xc_width = CGraphics::g_CroppedViewport.xc_width - clipRect.x4_left;
    m_uniform.m_indXf[0][0] = clipRect.xc_width / oldW;
  }

  tmp = clipRect.x8_top + clipRect.x10_height;
  if (tmp >= CGraphics::g_CroppedViewport.x10_height) {
    clipRect.x10_height = CGraphics::g_CroppedViewport.x10_height - clipRect.x8_top;
    m_uniform.m_indXf[1][1] = clipRect.x10_height / oldH;
  }

  /* Transform UV coordinates of rectangle within viewport and sampled scene texels (clamped to viewport bounds) */
  zeus::CVector2f vp{float(CGraphics::g_CroppedViewport.xc_width), float(CGraphics::g_CroppedViewport.x10_height)};
  m_uniform.m_matrix[0][0] = clipRect.xc_width / vp.x();
  m_uniform.m_matrix[1][1] = clipRect.x10_height / vp.y();
  m_uniform.m_matrix[3][0] = pt.x() + (1.f / vp.x());
  m_uniform.m_matrix[3][1] = pt.y() + (1.f / vp.y());
  //  if (CGraphics::g_BooPlatform == boo::IGraphicsDataFactory::Platform::OpenGL) {
  //    m_uniform.m_matrix[3][2] = pt.z() * 2.f - 1.f;
  //  } else if (CGraphics::g_BooPlatform == boo::IGraphicsDataFactory::Platform::Vulkan) {
  //    m_uniform.m_matrix[1][1] *= -1.f;
  //    m_uniform.m_matrix[3][1] *= -1.f;
  //    m_uniform.m_matrix[3][2] = pt.z();
  //  } else {
  m_uniform.m_matrix[3][2] = pt.z();
  //  }

  if (clipRect.x4_left) {
    clipRect.x4_left -= 1;
    clipRect.xc_width += 1;
  }
  if (clipRect.x8_top) {
    clipRect.x8_top -= 1;
    clipRect.x10_height += 1;
  }
  if (clipRect.x4_left + clipRect.xc_width < CGraphics::g_CroppedViewport.xc_width)
    clipRect.xc_width += 1;
  if (clipRect.x8_top + clipRect.x10_height < CGraphics::g_CroppedViewport.x10_height)
    clipRect.x10_height += 1;

  clipRect.x4_left += CGraphics::g_CroppedViewport.x4_left;
  clipRect.x8_top += CGraphics::g_CroppedViewport.x8_top;
  clipRect.x8_top = g_Viewport.xc_height - clipRect.x10_height - clipRect.x8_top;
  CGraphics::ResolveSpareTexture(clipRect);

  m_uniform.m_strength.x =
      m_uniform.m_matrix[0][0] * m_strength * 0.5f * (clipRect.x10_height / float(clipRect.xc_width));
  m_uniform.m_strength.y = m_uniform.m_matrix[1][1] * m_strength * 0.5f;
  m_uniBuf.load(m_uniform);

  m_dataBind.draw(0, 4);
}

} // namespace urde
