#include "Runtime/Graphics/Shaders/CSpaceWarpFilter.hpp"

#include "Runtime/Graphics/CCubeRenderer.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

//#include <hecl/Pipeline.hpp>

#define WARP_RAMP_RES 32

namespace metaforce {

// static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

void CSpaceWarpFilter::Initialize() {
  //  s_Pipeline = hecl::conv->convert(Shader_CSpaceWarpFilter{});
}

void CSpaceWarpFilter::Shutdown() {
  //  s_Pipeline.reset();
}

void CSpaceWarpFilter::GenerateWarpRampTex() {
  std::array<std::array<std::array<u8, 4>, WARP_RAMP_RES + 1>, WARP_RAMP_RES + 1> data{};
  const float halfRes = WARP_RAMP_RES / 2.f;
  for (int y = 0; y < WARP_RAMP_RES + 1; ++y) {
    for (int x = 0; x < WARP_RAMP_RES + 1; ++x) {
      zeus::CVector2f vec((x - halfRes) / halfRes, (y - halfRes) / halfRes);
      const float mag = vec.magnitude();
      if (mag < 1.f && vec.canBeNormalized()) {
        vec.normalize();
        vec *= zeus::CVector2f(std::sqrt(mag));
      }
      data[y][x][3] = zeus::clamp(0, int((((vec.x() / 2.f + 0.5f) - x / float(WARP_RAMP_RES)) + 0.5f) * 255), 255);
      data[y][x][2] = zeus::clamp(0, int((((vec.y() / 2.f + 0.5f) - y / float(WARP_RAMP_RES)) + 0.5f) * 255), 255);
      data[y][x][0] = data[y][x][1] = data[y][x][2];
    }
  }
  m_warpTex = aurora::gfx::new_static_texture_2d(
      WARP_RAMP_RES + 1, WARP_RAMP_RES + 1, 1, GX::TF_RGBA8,
      {reinterpret_cast<const uint8_t*>(data.data()), (WARP_RAMP_RES + 1) * (WARP_RAMP_RES + 1) * 4}, "Warp Ramp");
}

CSpaceWarpFilter::CSpaceWarpFilter() {
//  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
//    GenerateWarpRampTex(ctx);
//
//    struct Vert {
//      zeus::CVector2f m_pos;
//      zeus::CVector2f m_uv;
//    };
//    const std::array<Vert, 4> verts{{
//        {{-1.f, -1.f}, {0.f, 0.f}},
//        {{-1.f, 1.f}, {0.f, 1.f}},
//        {{1.f, -1.f}, {1.f, 0.f}},
//        {{1.f, 1.f}, {1.f, 1.f}},
//    }};
//
//    m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts.data(), 32, verts.size());
//    m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
//
//    const std::array<boo::ObjToken<boo::IGraphicsBuffer>, 1> bufs{m_uniBuf.get()};
//    constexpr std::array<boo::PipelineStage, 1> stages{boo::PipelineStage::Vertex};
//    const std::array<boo::ObjToken<boo::ITexture>, 2> texs{
//        CGraphics::g_SpareTexture.get(),
//        m_warpTex.get(),
//    };
//    m_dataBind = ctx.newShaderDataBinding(s_Pipeline, m_vbo.get(), nullptr, nullptr, bufs.size(), bufs.data(),
//                                          stages.data(), nullptr, nullptr, texs.size(), texs.data(), nullptr, nullptr);
//    return true;
//  } BooTrace);
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
//    m_uniform.m_matrix[3][2] = pt.z();
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
  clipRect.x8_top = CGraphics::GetViewportHeight() - clipRect.x10_height - clipRect.x8_top;
//  CGraphics::ResolveSpareTexture(clipRect);

  m_uniform.m_strength.x() =
      m_uniform.m_matrix[0][0] * m_strength * 0.5f * (clipRect.x10_height / float(clipRect.xc_width));
  m_uniform.m_strength.y() = m_uniform.m_matrix[1][1] * m_strength * 0.5f;
//  m_uniBuf->load(&m_uniform, sizeof(m_uniform));

//  CGraphics::SetShaderDataBinding(m_dataBind);
//  CGraphics::DrawArray(0, 4);
}

} // namespace metaforce
