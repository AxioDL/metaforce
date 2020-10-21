#include "Runtime/Graphics/Shaders/CRandomStaticFilter.hpp"

#include <array>

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"

#include "CRandomStaticFilter.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

template <EFilterType Type, bool CookieCutter>
struct FilterTypeAttachmentExt {
  using type = color_attachment<>;
};
template <bool CookieCutter>
struct FilterTypeAttachmentExt<EFilterType::Blend, CookieCutter> {
  using type = BlendAttachmentExt<CookieCutter ? AlphaMode::AlphaReplace : AlphaMode::NoAlpha>;
};
template <bool CookieCutter>
struct FilterTypeAttachmentExt<EFilterType::Add, CookieCutter> {
  using type = AdditiveAttachmentExt<CookieCutter ? AlphaMode::AlphaReplace : AlphaMode::NoAlpha>;
};
template <bool CookieCutter>
struct FilterTypeAttachmentExt<EFilterType::Multiply, CookieCutter> {
  using type = MultiplyAttachmentExt<CookieCutter ? AlphaMode::AlphaReplace : AlphaMode::NoAlpha>;
};
template <bool CookieCutter>
struct FilterTypeAttachmentExt<EFilterType::NoColor, CookieCutter> {
  using type = NoColorAttachmentExt<CookieCutter ? AlphaMode::AlphaReplace : AlphaMode::NoAlpha>;
};
template <EFilterType Type, bool CookieCutter>
using FilterTypeAttachment = typename FilterTypeAttachmentExt<Type, CookieCutter>::type;

template <EFilterType Type, bool CookieCutter>
struct CRandomStaticFilterPipeline
: pipeline<topology<hsh::TriangleStrip>, FilterTypeAttachment<Type, CookieCutter>,
           depth_compare<CookieCutter ? hsh::LEqual : hsh::Always>, depth_write<CookieCutter>> {
  CRandomStaticFilterPipeline(hsh::vertex_buffer<CRandomStaticFilter::Vert> vbo,
                              hsh::uniform_buffer<CRandomStaticFilter::Uniform> ubo, hsh::texture2d tex) {
    this->position = hsh::float4(vbo->m_pos, 0.f, 1.f);
    hsh::float2 uv HSH_VAR_STAGE(fragment) = vbo->m_uv;
    hsh::float4 color = tex.read<float>(Lookup8BPP(uv, ubo->randOff)) * ubo->color;
    if constexpr (CookieCutter) {
      if (color.w < ubo->discardThres) {
        hsh::discard();
      }
    } else {
      color.w = ubo->color.w;
    }
    this->color_out[0] = color;
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
template struct CRandomStaticFilterPipeline<EFilterType::Blend, false>;
template struct CRandomStaticFilterPipeline<EFilterType::Add, false>;
template struct CRandomStaticFilterPipeline<EFilterType::Multiply, false>;
template struct CRandomStaticFilterPipeline<EFilterType::NoColor, true>;

CRandomStaticFilter::CRandomStaticFilter(EFilterType type, bool cookieCutter) : m_cookieCutter(cookieCutter) {
  constexpr std::array<Vert, 4> verts{{
      {{-1.f, -1.f}, {0.f, 0.f}},
      {{-1.f, 1.f}, {0.f, 448.f}},
      {{1.f, -1.f}, {640.f, 0.f}},
      {{1.f, 1.f}, {640.f, 448.f}},
  }};
  m_vbo = hsh::create_vertex_buffer(verts);
  m_uniBuf = hsh::create_dynamic_uniform_buffer<Uniform>();
  m_dataBind.hsh_bind(CRandomStaticFilterPipeline<type, cookieCutter>(m_vbo.get(), m_uniBuf.get(),
                                                                      g_Renderer->GetRandomStaticEntropyTex()));
}

void CRandomStaticFilter::draw(const zeus::CColor& color, float t) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CRandomStaticFilter::draw", zeus::skMagenta);

  m_uniform.color = color;
  m_uniform.randOff = ROUND_UP_32(int64_t(rand()) * 32767 / RAND_MAX);
  m_uniform.discardThres = 1.f - t;
#if !HSH_PROFILE_MODE
  m_uniBuf.load(m_uniform);
#endif

  m_dataBind.draw(0, 4);
}

} // namespace urde
